#pragma once
#include <SimuCore/SimuCoreWebsocketServer.hpp>
#include <memory>

// Null implementation of the SimuCoreWebsocketServer
class NoImplementationWebsocketServer : public SimuCoreWebsocketServer {
public:
    // Core interface methods (no-op implementations)
    bool start(int port) override {
        return false; // Always "fails" to start
    }

    void stop() override {
        // Do nothing
    }

    bool is_running() const override {
        return false; // Always not running
    }

    bool send_message_to_connected_clients(const std::string& /*message*/) override {
        return false; // No clients to send to
    }

    void broadcast_message(const std::string& /*message*/) override {
        // Do nothing
    }

    // Callback setters (ignore the callbacks)
    void set_message_callback(MessageCallback /*callback*/) override {
        // Do nothing
    }

    void set_connection_callback(ConnectionCallback /*callback*/) override {
        // Do nothing
    }
};