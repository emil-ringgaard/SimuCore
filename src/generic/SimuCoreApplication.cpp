#include <SimuCore/generated/Config.hpp>
#include <SimuCore/SimuCoreApplication.hpp>

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
        websocket_server_->send_message_to_client(clientId, this->buildComponentTree().dump());
    }
}

void SimuCoreApplication::on_message(int clientId, const std::string &message)
{
    // Implementation commented out
}

nlohmann::json SimuCoreApplication::buildComponentTree()
{
    auto buildJsonTree = [](Component *component, auto &self) -> nlohmann::json
    {
        nlohmann::json componentJson;
        componentJson["name"] = component->getName();
        componentJson["id"] = component->getId();

        if (component->getComponentType() == ComponentType::INTERNAL_INPUT ||
            component->getComponentType() == ComponentType::INTERNAL_OUTPUT)
        {
            const auto signal = SignalRegistry::getInstance().find(component->getId());

            componentJson["value"] = signal->getValueAsString();
            componentJson["typeName"] = signal->getTypeName();

            if (component->getComponentType() == ComponentType::INTERNAL_OUTPUT)
            {
                for (auto *connectedInput : signal->getConnectedBaseSignals())
                {
                    if (componentJson.find("connectedInputs") == componentJson.end())
                    {
                        componentJson["connectedInputs"] = nlohmann::json::array();
                    }
                    componentJson["connectedInputs"].push_back(
                        nlohmann::json{{"id", connectedInput->getId()}});
                }
            }
        }

        auto subComponents = component->getSubComponents();
        if (!subComponents.empty())
        {
            for (auto *subComponent : subComponents)
            {
                if (subComponent)
                {
                    std::string componentType = subComponent->getComponentTypeName() + "s";
                    if (componentJson.find(componentType) == componentJson.end())
                    {
                        componentJson[componentType] = nlohmann::json::array();
                    }
                    componentJson[componentType].push_back(self(subComponent, self));
                }
            }
        }

        return componentJson;
    };

    return buildJsonTree(this, buildJsonTree);
}

void SimuCoreApplication::initApp()
{
    bindSignals();
    initAll();
}

void SimuCoreApplication::run()
{
    executeAll();
    sendSignalValuesToWebsockets();
    simu_core_tick->wait_for_next_tick();
}

void SimuCoreApplication::sendSignalValuesToWebsockets()
{
    // Implementation commented out
}

void SimuCoreApplication::init()
{
}

void SimuCoreApplication::execute()
{
    // Application's execute is called first, then all subcomponents
}
