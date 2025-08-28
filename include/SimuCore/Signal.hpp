#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <typeinfo>
#include <iostream>
#include <SimuCore/Component.hpp>
#include <SimuCore/json.hpp>

// ------------------------------------------------------------
// Common result type for setting values from string
// ------------------------------------------------------------
enum class SetValueByStringResult
{
	ReadOnly,
	UnsupportedType,
	Success
};

struct SetValueResponse
{
	SetValueByStringResult result;
	std::string message;
};

// ------------------------------------------------------------
// SignalBase
// ------------------------------------------------------------
class SignalBase : public Component
{
public:
	SignalBase(Component *owner, const std::string &name, ComponentType componentType)
		: Component(owner, name, componentType) {}
	virtual ~SignalBase() = default;

	virtual void registerSignal() = 0;
	virtual std::string getTypeName() const = 0;
	virtual std::string getValueAsString() const = 0;
	virtual SetValueResponse setValueFromString(const std::string &value) = 0;
	virtual bool valueHasChanged() = 0;

	void init() override {}
	void execute() override {}

	std::vector<SignalBase *> getConnectedBaseSignals() const { return connectedBaseSignals_; }
	void addBaseSignal(SignalBase *signal) { connectedBaseSignals_.push_back(signal); }

protected:
	std::vector<SignalBase *> connectedBaseSignals_;
};

// ------------------------------------------------------------
// Unified Signal<T>
// ------------------------------------------------------------
template <typename T>
class Signal : public SignalBase
{
public:
	Signal(Component *owner, const std::string &name,
		   ComponentType componentType, const T &initial_value = T{})
		: SignalBase(owner, name, componentType),
		  value_(initial_value), last_value_(initial_value)
	{
		registerSignal();
	}

	// Value access
	virtual void setValue(const T &value)
	{
		last_value_ = value_;
		value_ = value;
	}
	const T &getValue() const { return value_; }

	// Introspection
	std::string getTypeName() const override { return typeid(T).name(); }

	std::string getValueAsString() const override
	{
		if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, double>)
			return std::to_string(value_);
		else if constexpr (std::is_same_v<T, bool>)
			return value_ ? "true" : "false";
		else if constexpr (std::is_same_v<T, std::string>)
			return value_;
		else
			return "Unsupported type";
	}

	SetValueResponse setValueFromString(const std::string &value) override
	{
		// Parameters & physical I/O can be set, but internal signals are read-only
		if (getComponentType() != ComponentType::PHYSICAL_INPUT &&
			getComponentType() != ComponentType::PHYSICAL_OUTPUT &&
			getComponentType() != ComponentType::PARAMETER)
		{
			return {SetValueByStringResult::ReadOnly,
					"Cannot set value! Only Physical I/O and Parameters are writable"};
		}

		try
		{
			if constexpr (std::is_same_v<T, int>)
				setValue(std::stoi(value));
			else if constexpr (std::is_same_v<T, bool>)
				setValue(value == "true");
			else if constexpr (std::is_same_v<T, float>)
				setValue(std::stof(value));
			else if constexpr (std::is_same_v<T, double>)
				setValue(std::stod(value));
			else if constexpr (std::is_same_v<T, std::string>)
				setValue(value);
			else
				return {SetValueByStringResult::UnsupportedType, "Unsupported type"};
		}
		catch (...)
		{
			return {SetValueByStringResult::UnsupportedType, "Conversion failed"};
		}

		return {SetValueByStringResult::Success, "Success"};
	}

	bool valueHasChanged() override
	{
		if (first_read_)
		{
			first_read_ = false;
			return true;
		}
		return value_ != last_value_;
	}

	void registerSignal() override;

protected:
	T value_;
	T last_value_;
	bool first_read_ = true;
};

// ------------------------------------------------------------
// Specializations as "aliases"
// ------------------------------------------------------------
template <typename T>
class InputSignal : public Signal<T>
{
public:
	InputSignal(Component *owner, const std::string &name, const T &initial_value = T{})
		: Signal<T>(owner, name, ComponentType::INTERNAL_INPUT, initial_value) {}
};

template <typename T>
class OutputSignal : public Signal<T>
{
private:
	std::vector<InputSignal<T> *> connected_inputs_;

public:
	OutputSignal(Component *owner, const std::string &name, const T &initial_value = T{})
		: Signal<T>(owner, name, ComponentType::INTERNAL_OUTPUT, initial_value) {}

	void setValue(const T &value) override
	{
		this->last_value_ = this->value_;
		this->value_ = value;
		for (auto *input : connected_inputs_)
			input->setValue(this->value_);
	}

	void connectTo(InputSignal<T> *input)
	{
		connected_inputs_.push_back(input);
		input->setValue(this->value_);
		this->addBaseSignal(input);
	}
};

template <typename T>
class PhysicalInput : public Signal<T>
{
public:
	PhysicalInput(Component *owner, const std::string &name, const T &initial_value = T{})
		: Signal<T>(owner, name, ComponentType::PHYSICAL_INPUT, initial_value) {}
};

template <typename T>
class PhysicalOutput : public Signal<T>
{
public:
	PhysicalOutput(Component *owner, const std::string &name, const T &initial_value = T{})
		: Signal<T>(owner, name, ComponentType::PHYSICAL_OUTPUT, initial_value) {}
};

template <typename T>
class Parameter : public Signal<T>
{
public:
	Parameter(Component *owner, const std::string &name, const T &initial_value = T{})
		: Signal<T>(owner, name, ComponentType::PARAMETER, initial_value) {}
};

// ------------------------------------------------------------
// Signal Registry
// ------------------------------------------------------------
class SignalRegistry
{
public:
	static SignalRegistry &getInstance()
	{
		static SignalRegistry instance;
		return instance;
	}

	const SignalBase *find(const uint id) const
	{
		auto it = signals_.find(id);
		return (it != signals_.end()) ? it->second : nullptr;
	}

	std::vector<SignalBase *> getAllSignals() const
	{
		std::vector<SignalBase *> allSignals;
		allSignals.reserve(signals_.size());
		for (auto &p : signals_)
			allSignals.push_back(p.second);
		return allSignals;
	}

	void changeSignalValue(uint id, const std::string &value)
	{
		auto it = signals_.find(id);
		if (it != signals_.end())
		{
			auto response = it->second->setValueFromString(value);
			SimuCoreLogger::log(response.message);
		}
	}

private:
	SignalRegistry() = default;
	SignalRegistry(const SignalRegistry &) = delete;
	SignalRegistry &operator=(const SignalRegistry &) = delete;

	void add(SignalBase *signal) { signals_[signal->getId()] = signal; }

	std::unordered_map<uint, SignalBase *> signals_;

	template <typename T>
	friend class Signal;
};

// ------------------------------------------------------------
// Register signal implementation
// ------------------------------------------------------------
template <typename T>
void Signal<T>::registerSignal()
{
	SignalRegistry::getInstance().add(this);
}
