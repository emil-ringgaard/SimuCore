#include <SimuCore/SimuCoreBaseConfig.hpp>
#include <string>

class SimuCoreLogger 
{
	public:
	static void log(std::string message) 
	{
		if (config_instance.log_enabled) 
		{
			log_(message);	
		}
	}
	private:
	static void log_(std::string message);
};