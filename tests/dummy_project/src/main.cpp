#include <Application.hpp>
#include <SimuCore/generated/Config.hpp>
#include <memory>
#include <iostream>
Application *application = new Application();

void setup()
{
	application->initApp();
}

void loop()
{
	application->run();
	// SimuCoreLogger::log(application->hellp.physical_input_signal.getValueAsString());
}
