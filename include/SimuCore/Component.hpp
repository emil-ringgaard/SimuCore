#pragma once
#include <vector>
#include <string>
#include <memory>

enum class ComponentType 
{
	Component,
	InputSignal,
	OutputSignal
};

class Component
{
protected:
	std::string name_;
	std::vector<Component *> subcomponents_;
	Component *parent_;
	ComponentType componentType_;
	void setComponentType(const ComponentType componentType) 
	{
		componentType_ = componentType;
	}

public:
	Component(Component *parent, const std::string &name, ComponentType componentType = ComponentType::Component) : name_(name), parent_(parent), componentType_(componentType)

	{
		if (parent)
		{
			parent->subcomponents_.push_back(this);
		}
	}
	virtual void init() = 0;
	virtual void execute() = 0;
	std::string getFullName() 
	{
		if (parent_)
		{
			return parent_->getName() + "->" + name_;
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
		for (auto* sub : subcomponents_) {
			sub->initAll();
		}
	}
	void executeAll() {
		execute();
		
		// Execute all subcomponents
		for (auto* sub : subcomponents_) {
			sub->executeAll();
		}
	}
};
