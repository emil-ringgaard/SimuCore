
#pragma once
#include <string>
#include <functional>
#include <memory>

// Abstract interface for WebSocket server
class SimuCoreWebsocketServer
{
public:
	// Callback types
	using MessageCallback = std::function<void(int client_id, const std::string &message)>;
	using ConnectionCallback = std::function<void(int client_id, bool connected)>;

	virtual ~SimuCoreWebsocketServer() = default;

	// Core interface methods
	virtual bool start(int port) = 0;
	virtual void stop() = 0;
	virtual bool is_running() const = 0;
	virtual bool send_message_to_connected_clients(const std::string &message) = 0;
	virtual bool send_message_to_client(int clientId, const std::string &message) = 0;
	virtual void broadcast_message(const std::string &message) = 0;

	// Callback setters
	virtual void set_message_callback(MessageCallback callback) = 0;
	virtual void set_connection_callback(ConnectionCallback callback) = 0;
	static std::unique_ptr<SimuCoreWebsocketServer> create_websocket_server();
};