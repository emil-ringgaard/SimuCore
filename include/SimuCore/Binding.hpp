#pragma once
#include <SimuCore/Signal.hpp>
#include <SimuCore/SimuCoreLogger.hpp>

class ComponentBinder
{
public:
	template <typename T>
	static void bind(OutputSignal<T> &output, InputSignal<T> &input)
	{
		output.connectTo(&input);
		SimuCoreLogger::log("Bound " + output.getFullName() + "to " + input.getFullName());
	}
};
