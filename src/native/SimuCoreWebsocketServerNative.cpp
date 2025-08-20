#include <SimuCore/SimuCoreWebsocketServer.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include <thread>
#include <atomic>
#include <map>
#include <mutex>

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "ws2_32.lib")
	typedef int socklen_t;
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#define closesocket close
	typedef int SOCKET;
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
#endif

class SimuCoreWebsocketServerNative : public SimuCoreWebsocketServer {
private:
	SOCKET server_sock;
	int port;
	std::atomic<bool> running;
	std::thread server_thread;
	
	// Client management
	std::map<int, SOCKET> clients;
	std::mutex clients_mutex;
	int next_client_id;
	
	// Callbacks
	MessageCallback message_callback;
	ConnectionCallback connection_callback;
	
	std::string base64_encode(const std::string& input) {
		const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		std::string result;
		int val = 0, valb = -6;
		for (unsigned char c : input) {
			val = (val << 8) + c;
			valb += 8;
			while (valb >= 0) {
				result.push_back(chars[(val >> valb) & 0x3F]);
				valb -= 6;
			}
		}
		if (valb > -6) result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
		while (result.size() % 4) result.push_back('=');
		return result;
	}
	
	std::string sha1_hash(const std::string& input) {
		uint32_t h[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
		
		std::string padded = input;
		padded += '\x80';
		while ((padded.length() % 64) != 56) {
			padded += '\x00';
		}
		
		uint64_t bit_len = input.length() * 8;
		for (int i = 7; i >= 0; i--) {
			padded += static_cast<char>((bit_len >> (i * 8)) & 0xFF);
		}
		
		for (size_t chunk = 0; chunk < padded.length(); chunk += 64) {
			uint32_t w[80];
			for (int i = 0; i < 16; i++) {
				w[i] = (static_cast<uint32_t>(static_cast<unsigned char>(padded[chunk + i*4])) << 24) |
					   (static_cast<uint32_t>(static_cast<unsigned char>(padded[chunk + i*4 + 1])) << 16) |
					   (static_cast<uint32_t>(static_cast<unsigned char>(padded[chunk + i*4 + 2])) << 8) |
					   static_cast<uint32_t>(static_cast<unsigned char>(padded[chunk + i*4 + 3]));
			}
			
			for (int i = 16; i < 80; i++) {
				w[i] = w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16];
				w[i] = (w[i] << 1) | (w[i] >> 31);
			}
			
			uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
			
			for (int i = 0; i < 80; i++) {
				uint32_t f, k;
				if (i < 20) {
					f = (b & c) | (~b & d);
					k = 0x5A827999;
				} else if (i < 40) {
					f = b ^ c ^ d;
					k = 0x6ED9EBA1;
				} else if (i < 60) {
					f = (b & c) | (b & d) | (c & d);
					k = 0x8F1BBCDC;
				} else {
					f = b ^ c ^ d;
					k = 0xCA62C1D6;
				}
				
				uint32_t temp = ((a << 5) | (a >> 27)) + f + e + k + w[i];
				e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
			}
			
			h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e;
		}
		
		std::string result;
		for (int i = 0; i < 5; i++) {
			for (int j = 3; j >= 0; j--) {
				result += static_cast<char>((h[i] >> (j * 8)) & 0xFF);
			}
		}
		return result;
	}
	
	std::string generate_accept_key(const std::string& client_key) {
		std::string combined = client_key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
		return base64_encode(sha1_hash(combined));
	}
	
	void handle_client(SOCKET client_sock, int client_id) {
		// Read HTTP handshake request
		char buffer[4096];
		int bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
		if (bytes_received <= 0) {
			closesocket(client_sock);
			return;
		}
		
		buffer[bytes_received] = '\0';
		std::string request(buffer);
		
		// Extract the WebSocket key from the request
		std::string key_header = "Sec-WebSocket-Key: ";
		size_t key_pos = request.find(key_header);
		
		if (key_pos == std::string::npos) {
			closesocket(client_sock);
			return;
		}
		
		key_pos += key_header.length();
		size_t key_end = request.find("\r\n", key_pos);
		std::string client_key = request.substr(key_pos, key_end - key_pos);
		
		// Generate proper WebSocket accept key
		std::string accept_key = generate_accept_key(client_key);
		
		// WebSocket handshake response
		std::string response = 
			"HTTP/1.1 101 Switching Protocols\r\n"
			"Upgrade: websocket\r\n"
			"Connection: Upgrade\r\n"
			"Sec-WebSocket-Accept: " + accept_key + "\r\n"
			"\r\n";
		
		send(client_sock, response.c_str(), response.length(), 0);
		
		// Notify connection
		if (connection_callback) {
			connection_callback(client_id, true);
		}
		
		// Handle WebSocket communication
		while (running.load()) {
			// Read frame header
			unsigned char header[2];
			int received = recv(client_sock, (char*)header, 2, 0);
			if (received != 2) break;
			
			bool fin = (header[0] & 0x80) != 0;
			unsigned char opcode = header[0] & 0x0F;
			bool masked = (header[1] & 0x80) != 0;
			unsigned char payload_len = header[1] & 0x7F;
			
			// Handle extended payload length
			uint64_t extended_len = payload_len;
			if (payload_len == 126) {
				unsigned char len_bytes[2];
				if (recv(client_sock, (char*)len_bytes, 2, 0) != 2) break;
				extended_len = (len_bytes[0] << 8) | len_bytes[1];
			} else if (payload_len == 127) {
				break; // Skip large payloads for simplicity
			}
			
			// Read masking key (if present)
			unsigned char mask[4] = {0};
			if (masked) {
				if (recv(client_sock, (char*)mask, 4, 0) != 4) break;
			}
			
			// Read payload
			std::string payload;
			if (extended_len > 0) {
				payload.resize(extended_len);
				int payload_received = recv(client_sock, &payload[0], extended_len, 0);
				if (payload_received != (int)extended_len) break;
				
				// Unmask payload
				if (masked) {
					for (uint64_t i = 0; i < extended_len; i++) {
						payload[i] ^= mask[i % 4];
					}
				}
			}
			
			// Handle different frame types
			if (opcode == 0x1) { // Text frame
				if (message_callback) {
					message_callback(client_id, payload);
				}
			} else if (opcode == 0x8) { // Close frame
				break;
			} else if (opcode == 0x9) { // Ping frame
				send_pong_frame(client_sock, payload);
			}
		}
		
		// Clean up client
		{
			std::lock_guard<std::mutex> lock(clients_mutex);
			clients.erase(client_id);
		}
		
		if (connection_callback) {
			connection_callback(client_id, false);
		}
		
		closesocket(client_sock);
	}
	
	void send_text_frame(SOCKET client_sock, const std::string& message) {
		std::vector<unsigned char> frame;
		frame.push_back(0x81); // FIN=1, opcode=1 (text)
		
		size_t payload_len = message.length();
		if (payload_len < 126) {
			frame.push_back(payload_len);
		} else if (payload_len < 65516) {
			frame.push_back(126);
			frame.push_back((payload_len >> 8) & 0xFF);
			frame.push_back(payload_len & 0xFF);
		}
		
		// Add payload (no masking for server)
		for (char c : message) {
			frame.push_back(c);
		}
		
		send(client_sock, (char*)frame.data(), frame.size(), 0);
	}
	
	void send_pong_frame(SOCKET client_sock, const std::string& payload) {
		std::vector<unsigned char> frame;
		frame.push_back(0x8A); // FIN=1, opcode=10 (pong)
		frame.push_back(payload.length());
		
		for (char c : payload) {
			frame.push_back(c);
		}
		
		send(client_sock, (char*)frame.data(), frame.size(), 0);
	}
	
	void server_loop() {
		while (running.load()) {
			struct sockaddr_in client_addr;
			socklen_t client_len = sizeof(client_addr);
			
			SOCKET client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
			if (client_sock == INVALID_SOCKET) {
				if (running.load()) continue;
				break;
			}
			
			int client_id = next_client_id++;
			
			{
				std::lock_guard<std::mutex> lock(clients_mutex);
				clients[client_id] = client_sock;
			}
			
			std::thread client_thread(&SimuCoreWebsocketServerNative::handle_client, this, client_sock, client_id);
			client_thread.detach();
		}
	}

public:
	SimuCoreWebsocketServerNative() : server_sock(INVALID_SOCKET), port(0), running(false), next_client_id(1) {
#ifdef _WIN32
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
	}
	
	virtual ~SimuCoreWebsocketServerNative() {
		stop();
#ifdef _WIN32
		WSACleanup();
#endif
	}
	
	bool start(int p) override {
		if (running.load()) return false;
		
		port = p;
		
		// Create socket
		server_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (server_sock == INVALID_SOCKET) return false;
		
		// Allow socket reuse
		int opt = 1;
		setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
		
		// Bind socket
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = INADDR_ANY;
		
		if (bind(server_sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
			closesocket(server_sock);
			return false;
		}
		
		if (listen(server_sock, 5) == SOCKET_ERROR) {
			closesocket(server_sock);
			return false;
		}
		
		running.store(true);
		server_thread = std::thread(&SimuCoreWebsocketServerNative::server_loop, this);
		
		std::cout << "Linux WebSocket server started on port " << port << std::endl;
		return true;
	}
	
	void stop() override {
		if (running.load()) {
			running.store(false);
			
			if (server_sock != INVALID_SOCKET) {
				closesocket(server_sock);
				server_sock = INVALID_SOCKET;
			}
			
			if (server_thread.joinable()) {
				server_thread.join();
			}
			
			// Close all client connections
			std::lock_guard<std::mutex> lock(clients_mutex);
			for (auto& client : clients) {
				closesocket(client.second);
			}
			clients.clear();
		}
	}
	
	bool is_running() const override {
		return running.load();
	}
	
	bool send_message_to_connected_clients(const std::string& message) override {
		std::lock_guard<std::mutex> lock(clients_mutex);
		if (clients.size() <= 0) return false;
		for (auto &client : clients) {
			send_text_frame(client.second, message);
		}
		return true;
	}
	
	void broadcast_message(const std::string& message) override {
		std::lock_guard<std::mutex> lock(clients_mutex);
		for (const auto& client : clients) {
			send_text_frame(client.second, message);
		}
	}
	
	void set_message_callback(MessageCallback callback) override {
		message_callback = callback;
	}
	
	void set_connection_callback(ConnectionCallback callback) override {
		connection_callback = callback;
	}
};

// Factory function implementation for Linux/Windows
std::unique_ptr<SimuCoreWebsocketServer> SimuCoreWebsocketServer::create_websocket_server() {
	return std::make_unique<SimuCoreWebsocketServerNative>();
}