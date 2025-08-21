#pragma once
#include <vector>
#include <string>
#include <memory>
#include <iostream>

class BaseSignal;

class Component
{
protected:
	std::string name_;
	std::vector<Component *> subcomponents_;
	Component *parent_;

public:
	Component(Component *parent, const std::string &name) : name_(name), parent_(parent)

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
		for (auto *sub : subcomponents_)
		{
			sub->initAll();
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
};
