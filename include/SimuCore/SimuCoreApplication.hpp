#pragma once

#include "SimuCore/SimuCoreHAL.hpp"
#include <SimuCore/SimuCoreTick.hpp>
#include <SimuCore/SimuCoreLogger.hpp>
#include <SimuCore/generated/Config.hpp>
#include <SimuCore/Component.hpp>
#include <SimuCore/Signal.hpp>
#include <SimuCore/SimuCoreWebsocketServer.hpp>
#include <SimuCore/NoImplementationWebsocketServer.hpp>
#include <SimuCore/json.hpp>
#include <memory>
#include <string>

void to_json(nlohmann::json &j, SignalBase *signal);

class SimuCoreApplication : public Component
{
public:
	SimuCoreApplication(const std::string &applicationName);

	void initApp();
	void run();
	virtual void bindSignals() = 0;

protected:
	void sendSignalValuesToWebsockets();
	nlohmann::json buildComponentTree();

private:
	void on_connection(int clientId, bool connected);
	void on_message(int clientId, const std::string &message);

	void init() override;
	void execute() override;

	std::unique_ptr<SimuCoreHAL> hal;
	std::unique_ptr<SimuCoreTick> simu_core_tick;
	std::unique_ptr<SimuCoreWebsocketServer> websocket_server_;
};
