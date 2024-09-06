#pragma once

//#define PRINT_COMPONENT_ID

#ifdef PRINT_COMPONENT_ID
#include <tools/utility.hpp>
#include <iostream>
#endif

#include <commonTypes/componentId.hpp>

#include <functional>

enum class ComponentState { Ongoing, Changed, Outdated };

struct ComponentBase
{
#ifdef PRINT_COMPONENT_ID
	ComponentBase()
	{
		static bool init = true;
		if (init)
		{
			Tools::RedirectIOToConsole({ 2000, 10 });
			init = false;
		}
		std::cout << componentId << std::endl;
	}
#else
	ComponentBase() = default;
#endif

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
	ComponentState state = ComponentState::Changed;

private:
	ComponentId componentId = 0;
	bool enabled_ = true;
	bool static_ = true;
};
