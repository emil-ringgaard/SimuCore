#pragma once
#include <string>
#include <vector>
#include <SimuCore/Component.hpp>
#include <type_traits>


template <typename T>
class Signal : public Component
{
public:
	
	Signal(Component *owner, const std::string &name, ComponentType componentType, const T &initial_value = T{}) : value_(initial_value), owner_(owner), Component(owner, name, componentType)
	{
	}
	virtual void setValue(const T &value) = 0;
	void execute() override
	{
	}
	const T &getValue() const { return value_; }

protected:
	T value_;
	Component *owner_;
};

template <typename T>
class InputSignal : public Signal<T>
{
public:
	// Explicitly define constructor instead of using inheritance
	InputSignal(std::nullptr_t, const std::string &, const T & = T{}) = delete;
	InputSignal(Component *owner, const std::string &name, const T &initial_value = T{})
		: Signal<T>(owner, name, ComponentType::InputSignal, initial_value)
	{
	}

	void init() override
	{
	}

	void setValue(const T &value) override { this->value_ = value; }
};

template <typename T>
class OutputSignal : public Signal<T>
{
private:
	std::vector<InputSignal<T> *> connected_inputs_;

public:
	// Explicitly define constructor
	OutputSignal(std::nullptr_t, const std::string &, const T & = T{}) = delete;
	OutputSignal(Component *owner, const std::string &name, const T &initial_value = T{})
		: Signal<T>(owner, name, ComponentType::OutputSignal, initial_value)
	{
	}

	void setValue(const T &value) override
	{
		this->value_ = value;
		// Propagate to all connected inputs
		for (auto *input : connected_inputs_)
		{
			input->setValue(this->value_);
		}
	}
	
	void init() override 
	{
		
	}

	void connectTo(InputSignal<T> *input)
	{
		connected_inputs_.push_back(input);
		input->setValue(this->value_);
	}
};