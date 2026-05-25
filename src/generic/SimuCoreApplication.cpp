#include <SimuCore/generated/Config.hpp>
#include <SimuCore/SimuCoreApplication.hpp>
#include <unordered_set>
#include <string>

void to_json(nlohmann::json &j, SignalBase *signal)
{
    j = nlohmann::json{
        {"id", signal->getId()},
        {"name", signal->getName()},
        {"value", signal->getValueAsString()},
        {"typeName", signal->getTypeName()}};
}

SimuCoreApplication::SimuCoreApplication(const std::string &applicationName)
    : Component(nullptr, applicationName),
      hal(SimuCoreHAL::create()),
      simu_core_tick(SimuCoreTick::create()),
      _applicationTree(this),
      websocket_server_(SimuCore::config.enable_webserver.getValue()
                            ? SimuCoreWebsocketServer::create_websocket_server()
                            : std::make_unique<NoImplementationWebsocketServer>())
{
    websocket_server_->start(8080);
    websocket_server_->set_connection_callback([this](int clientId, bool connected)
                                               { this->on_connection(clientId, connected); });
    websocket_server_->set_message_callback([this](int clientId, const std::string &message)
                                            { this->on_message(clientId, message); });
}

void SimuCoreApplication::on_connection(int clientId, bool connected)
{
    if (connected)
    {
        websocket_server_->send_message_to_client(clientId, _applicationTreeJson.dump());
    }
}

void SimuCoreApplication::on_message(int clientId, const std::string &message)
{
    nlohmann::json jsonMsg = nlohmann::json::parse(message);
    if (!jsonMsg.contains("command"))
    {
        SimuCore::Response errorResponse{
            .message = "Received message did not contain a command!",
            .status = SimuCore::StatusEnum::FAILURE};
        nlohmann::json errorResponseJson = errorResponse;
        websocket_server_->send_message_to_client(clientId, errorResponseJson.dump());
        return;
    }
    SimuCore::Response successResponse;
    successResponse.status = SimuCore::StatusEnum::SUCCESS;
    SimuCore::CommandEnum command = jsonMsg["command"];
    if (command == SimuCore::CommandEnum::SUBSCRIBE)
    {
        std::unordered_set<int> idsSubscribedTo;
        for (auto &item : this->subscriptions)
        {
            idsSubscribedTo.insert(item.id);
        }
        SimuCore::SubscribeProtocol receivedSubscribeProtocol = jsonMsg;
        for (auto &signal : receivedSubscribeProtocol.payload)
        {
            if (idsSubscribedTo.count(signal.id))
            {
                successResponse.status = SimuCore::StatusEnum::WARNING;
                successResponse.message += "Signal " + std::to_string(signal.id) + " already exists in subscription list!";
            }
            else
            {
                subscriptions.push_back(signal);
                successResponse.message += "subscribed to: " + std::to_string(signal.id) + " ";
            }
        }
        websocket_server_->send_message_to_client(clientId, nlohmann::json(successResponse).dump());
    }
    else if (command == SimuCore::CommandEnum::START_SIMULATION)
    {
        simulation_system.is_simulating = true;
        this->reset_system();
        SimuCoreLogger::log("Starting simulation");
        websocket_server_->send_message_to_client(clientId, nlohmann::json(successResponse).dump());
    }
    else if (command == SimuCore::CommandEnum::STOP_SIMULATION)
    {
        simulation_system.is_simulating = false;
    }
    else if (command == SimuCore::CommandEnum::TICK)
    {
        SimuCore::TickSystem tick_system = jsonMsg;
        simulation_system.ticks_remaining = tick_system.number_of_ticks;
        return;
    }
    else if (command == SimuCore::CommandEnum::UPDATE_PHYSICAL_INPUT) {
        SimuCore::UpdatePysicalInputsProtocol update_inputs = jsonMsg;
        for (const auto &signal : update_inputs.parameters) {
            SignalRegistry::getInstance().changeSignalValue(signal.id, signal.value);
            auto new_signal = SignalRegistry::getInstance().find(signal.id);
        }
        websocket_server_->send_message_to_client(clientId, nlohmann::json(successResponse).dump());
    }
    else if (command == SimuCore::CommandEnum::INFO) {
        SimuCore::ApplicationInfoProtocol applicationInfo;
        applicationInfo.response = SimuCore::Response{.message = "Info", .status = SimuCore::StatusEnum::SUCCESS};
        applicationInfo.subscribed_signals = subscriptions;
        applicationInfo.up_time_in_milli_seconds = _up_time_in_milli_seconds;
        websocket_server_->send_message_to_client(clientId, nlohmann::json{applicationInfo}.dump());
    }
    else if (command == SimuCore::CommandEnum::APPLICATION_TREE) {
        websocket_server_->send_message_to_client(clientId, _applicationTree.getApplicationTreeAsJson().dump());
    }
}

void SimuCoreApplication::initApp()
{
    _applicationTreeJson = _applicationTree.getApplicationTreeAsJson();
    if (_initialApplicationTreeJson.is_null()) {
        _initialApplicationTreeJson = _applicationTreeJson;
    }
    _up_time_in_milli_seconds = 0;
    bindSignals();
    initAll();
}

void SimuCoreApplication::run()
{
    if (simulation_system.is_simulating.load())
    {
        // Wait for a TICK command to set ticks_remaining > 0
        while (simulation_system.ticks_remaining.load() == 0) { }
        
        executeAll();
        _up_time_in_milli_seconds += 1000 / SimuCore::config.sample_frequency.getValue();
        
        // Decrement and check if we just finished the batch
        int prev = simulation_system.ticks_remaining.fetch_sub(1);
        if (prev == 1) {
            SimuCore::Response successResponse;
            successResponse.status = SimuCore::StatusEnum::SUCCESS;
            websocket_server_->send_message_to_connected_clients(
                nlohmann::json(successResponse).dump());
        }
    }
    else
    {
        executeAll();
        _up_time_in_milli_seconds += 1000 / SimuCore::config.sample_frequency.getValue();
        sendSignalValuesToWebsockets();
        simu_core_tick->wait_for_next_tick();
    }
}

void SimuCoreApplication::sendSignalValuesToWebsockets()
{
    SimuCore::ApplicationInfoProtocol applicationInfo;
    applicationInfo.response = SimuCore::Response{.message = "Info", .status = SimuCore::StatusEnum::SUCCESS};
    applicationInfo.subscribed_signals = subscriptions;
    applicationInfo.up_time_in_milli_seconds = _up_time_in_milli_seconds;

    websocket_server_->send_message_to_connected_clients(nlohmann::json{applicationInfo}.dump());
}

void SimuCoreApplication::init()
{
}

void SimuCoreApplication::execute()
{
    // Application's execute is called first, then all subcomponents
}
void SimuCoreApplication::reset_system()
{
    _applicationTreeJson = _initialApplicationTreeJson;
    const auto signalRegistry = &SignalRegistry::getInstance();
    signalRegistry->reset_signals();
    this->initApp();
}