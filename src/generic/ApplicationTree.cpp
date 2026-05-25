#include <SimuCore/ApplicationTree.hpp>
#include <SimuCore/Signal.hpp>
#include <SimuCore/generated/Config.hpp>


const nlohmann::json ApplicationTree::getApplicationTreeAsJson()
{
    auto buildJsonTree = [](Component *component, auto &self) -> nlohmann::json
    {
        nlohmann::json componentJson;
        componentJson["name"] = component->getName();
        componentJson["id"] = component->getId();
        
        if (component->getComponentType() == ComponentType::INTERNAL_INPUT ||
            component->getComponentType() == ComponentType::INTERNAL_OUTPUT ||
            component->getComponentType() == ComponentType::PHYSICAL_INPUT ||
            component->getComponentType() == ComponentType::PHYSICAL_OUTPUT ||
            component->getComponentType() == ComponentType::PARAMETER)
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
    nlohmann::json applicationTree = buildJsonTree(_root, buildJsonTree);
    return applicationTree;
}