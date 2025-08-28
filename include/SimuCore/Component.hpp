#pragma once
#include <map>
#include <string>
#include <memory>
#include <iostream>
#include <SimuCore/SimuCoreLogger.hpp>

class SignalBase;

enum class ComponentType
{
	INTERNAL_INPUT,
	INTERNAL_OUTPUT,
	PHYSICAL_INPUT,
	PHYSICAL_OUTPUT,
	PARAMETER,
	COMPONENT
};

class Component
{
private:
	u_int32_t id_;

	int generateDeterministicId()
	{
		std::string full_path = this->getFullName();
		std::hash<std::string> hasher;
		int hash = static_cast<u_int32_t>(hasher(full_path));

		// Ensure ID is never 0 (reserve 0 for "invalid")
		return hash == 0 ? 1 : hash;
	}

protected:
	std::string name_;
	ComponentType componentType_;
	std::vector<Component *> subcomponents_;
	Component *parent_;

public:
	Component(Component *parent, const std::string &name, ComponentType componentType = ComponentType::COMPONENT) : name_(name), parent_(parent), componentType_(componentType)
	{
		if (parent)
		{
			parent->subcomponents_.push_back(this);
		}
		id_ = generateDeterministicId();
	}

	virtual void init() = 0;
	virtual void execute() = 0;
	std::string getFullName() const
	{
		if (parent_)
		{
			return parent_->getFullName() + "->" + name_;
		}
		return name_;
	}
	std::string getName()
	{
		return name_;
	}
	void initAll()
	{
		init();
		for (auto *sub : subcomponents_)
		{
			sub->initAll();
		}
	}
	uint32_t getId() const
	{
		return id_;
	}

	ComponentType getComponentType() const
	{
		return componentType_;
	}
	std::string getComponentTypeName() const
	{
		switch (componentType_)
		{
		case ComponentType::INTERNAL_INPUT:
			return "Input";
		case ComponentType::INTERNAL_OUTPUT:
			return "Output";
		case ComponentType::COMPONENT:
			return "Component";
		default:
			return "Unknown";
		}
	}
	void executeAll()
	{
		execute();

		// Execute all subcomponents
		for (auto *sub : subcomponents_)
		{
			sub->executeAll();
		}
	}
	std::vector<Component *> getSubComponents() const
	{
		return subcomponents_;
	}
};
