#pragma once
#include <string>

class SimuCoreLogger
{
public:
	static void log(const std::string &message);

private:
	static void log_(const std::string &message);
};