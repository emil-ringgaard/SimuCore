#pragma once
#include <string>
#include <vector>
#include <SimuCore/Component.hpp>
#include <type_traits>
#include <variant>
#include <iostream>
#include <typeinfo>

template <typename T>
class InputSignal;
template <typename T>
class OutputSignal;

template <typename T>
class Signal;

class SignalBase : public Component
{
public:
	virtual ~SignalBase() = default;
	SignalBase(Component *owner, const std::string &name) : Component(owner, name) {}
	virtual std::string getTypeName() const = 0;
	virtual std::string getValueAsString() const = 0;
	void init() override
	{
		// Initialization logic for the signal can go here if needed
	}

	void execute() override
	{
		// Execution logic for the signal can go here if needed
	}
};

template <typename T>
class Signal : public SignalBase
{
public:
	template <typename U>
	static constexpr auto get_converter()
	{
		if constexpr (std::is_same_v<U, int>)
		{
			return [](const U &val)
			{ return std::to_string(val); };
		}
		else if constexpr (std::is_same_v<U, bool>)
		{
			return [](const U &val)
			{ return val ? "true" : "false"; };
		}
		else if constexpr (std::is_same_v<U, float>)
		{
			return [](const U &val)
			{ return std::to_string(val); };
		}
		else if constexpr (std::is_same_v<U, double>)
		{
			return [](const U &val)
			{ return std::to_string(val); };
		}
		else if constexpr (std::is_same_v<U, std::string>)
		{
			return [](const U &val)
			{ return val; };
		}
		else
		{
			static_assert(std::is_same_v<U, void>, "Type not supported!");
			return nullptr;
		}
	}
	Signal(Component *owner, const std::string &name, const T &initial_value = T{}) : value_(initial_value), SignalBase(owner, name)
	{
		static_assert(!std::is_same_v<decltype(get_converter<T>()), std::nullptr_t>,
					  "Type not supported!");
	}
	virtual void setValue(const T &value) = 0;
	void execute() override
	{
	}

	const T &getValue() const { return value_; }
	std::string getTypeName() const override { return typeid(T).name(); }
	std::string getValueAsString() const override
	{
		auto converter = get_converter<T>();
		return converter ? converter(value_) : "Unsupported type";
	}

protected:
	T value_;
};

template <typename T>
class InputSignal : public Signal<T>
{
public:
	// Explicitly define constructor instead of using inheritance
	InputSignal(std::nullptr_t, const std::string &, const T & = T{}) = delete;
	InputSignal(Component *owner, const std::string &name, const T &initial_value = T{})
		: Signal<T>(owner, name, initial_value)
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
		: Signal<T>(owner, name, initial_value)
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