#pragma once

#include <componentId.hpp>

//#define PRINT_COMPONENT_ID

#ifdef PRINT_COMPONENT_ID
#include <iostream>
#include <tools/utility.hpp>
#endif

enum class ComponentState {Ongoing, Changed, Outdated};

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
#endif
	const ComponentId componentId = ComponentIdGenerator::instance().acquire();
	ComponentState state = ComponentState::Changed;
};
