#include <SimuCore/SimuCoreLogger.hpp>
#include <SimuCore/generated/Config.hpp>

void SimuCoreLogger::log(const std::string &message)
{
    if (SimuCore::config.log_enabled.getValue())
    {
        SimuCoreLogger::log_(message);
    }
}