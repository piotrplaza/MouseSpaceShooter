#pragma once

#include <commonTypes/componentId.hpp>

#include <functional>

enum class ComponentState { Ongoing, Changed, LastShot, Outdated };

struct ComponentBase
{
	ComponentBase() = default;

	virtual void init(ComponentId id, bool static_)
	{
		componentId = id;
		this->static_ = static_;
	}

	virtual void setEnabled(bool value)
	{
		enabled_ = value;
	}

	virtual bool isEnabled() const
	{
		return enabled_;
	}

	virtual void step()
	{
		if (stepF)
			stepF();
	}

	ComponentId getComponentId() const
	{
		return componentId;
	}

	bool isStatic() const
	{
		return static_;
	}

	std::function<void()> stepF;
	std::function<void()> teardownF;
	ComponentState state = ComponentState::Changed;

private:
	ComponentId componentId = 0;
	bool enabled_ = true;
	bool static_ = true;
};
