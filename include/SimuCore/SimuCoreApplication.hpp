#pragma once
// #if __cplusplus < 201703L
// #error "This library requires at least C++17. \
// Please add this to your platformio.ini:\
// build_unflags = -std=gnu++11 -std=gnu++14\
// build_flags = -std=gnu++17"
// #endif

#include "SimuCore/SimuCoreHAL.hpp"
#include <SimuCore/SimuCoreTick.hpp>
#include <SimuCore/SimuCoreBaseConfig.hpp>
#include <SimuCore/Component.hpp>
#include <SimuCore/SimuCoreWebsocketServer.hpp>
#include <SimuCore/NoImplementationWebsocketServer.hpp>

class SimuCoreApplication : public Component
{
public:
	SimuCoreApplication(const std::string &applicationName) : Component(nullptr,
																		applicationName),
															  hal(SimuCoreHAL::create()),
															  simu_core_tick(SimuCoreTick::create()),
															  websocket_server_(config_instance.enable_webserver ? SimuCoreWebsocketServer::create_websocket_server() : std::make_unique<NoImplementationWebsocketServer>())
	{
		websocket_server_.get()->start(8080);
	}
	
	void initApp() 
	{
		bindSignals();
		initAll();
	}

	// Run the entire application - executes all components in hierarchy
	void run()
	{
		executeAll();
		sendSignalValuesToWebsockets();
		simu_core_tick->wait_for_next_tick();
	}
	
	void sendSignalValuesToWebsockets() 
	{
		websocket_server_.get()->send_message_to_connected_clients("sending values!");
	}
	virtual void bindSignals() = 0;

private:
	void init() override
	{
		
	}
	void execute() override
	{
		// Application's execute is called first, then all subcomponents
	}
	std::unique_ptr<SimuCoreHAL> hal;
	std::unique_ptr<SimuCoreTick> simu_core_tick;
	std::unique_ptr<SimuCoreWebsocketServer> websocket_server_;
};
