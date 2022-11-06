#pragma once

//#define PRINT_COMPONENT_ID

#ifdef PRINT_COMPONENT_ID
#include <tools/utility.hpp>
#include <iostream>
#endif

#include <commonTypes/componentId.hpp>

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

	virtual void setComponentId(ComponentId id)
	{
		componentId = id;
	}

	virtual void enable(bool value)
	{
		enable_ = value;
	}

	virtual bool isEnabled() const
	{
		return enable_;
	}

	ComponentId getComponentId() const
	{
		return componentId;
	}

	ComponentState state = ComponentState::Changed;

private:
	ComponentId componentId = 0;
	bool enable_ = true;
};
