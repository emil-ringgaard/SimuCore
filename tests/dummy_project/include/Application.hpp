#pragma once
#include <SimuCore/SimuCoreApplication.hpp>
#include <SimuCore/Signal.hpp>
#include <SimuCore/Binding.hpp>
#include <iostream>

class AnotherTestComponent : public Component
{
public:
	AnotherTestComponent(Component *parent, std::string name) : Component(parent, name), input(this, "Some input")
	{
	}
	void execute()
	{
	}
	void init()
	{
	}
	InputSignal<int> input;
};

class TestComponent : public Component
{
public:
	TestComponent(Component *parent, std::string name) : Component(parent, name), testcomp(this, "TestComponent2"), output(this, "Someoutput")
	{
	}
	void execute()
	{
		static int i = 0;
		output.setValue(i++);
		output_double.setValue(3.14);
	}
	void init()
	{
	}

public:
	AnotherTestComponent testcomp;
	OutputSignal<int> output;
	OutputSignal<double> output_double{this, "OutputDouble", 3.14};
	OutputSignal<double> output_double2{this, "Din mor er en hest", 3.14};
	PhysicalInput<int> physical_input_signal{this, "Physical input signal", 2};
};

class Application : public SimuCoreApplication
{

public:
	Application() : SimuCoreApplication("Custom application name"), testcomp(this, "TestComponent")
	{
	}
	~Application() {}

	void execute()
	{
	}

	void bindSignals()
	{
		ComponentBinder::bind(testcomp.output, testcomp.testcomp.input);
	}

public:
	TestComponent testcomp;
	TestComponent hellp{
		this,
		"hihihihi"}; // Another test component to show that subcomponents can be created
};