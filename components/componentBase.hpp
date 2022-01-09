#pragma once

//#define PRINT_COMPONENT_ID

#ifdef PRINT_COMPONENT_ID
#include <tools/utility.hpp>
#include <iostream>
#endif

#include <globals/components.hpp>

enum class ComponentState {Ongoing, Changed, Outdated, Fake};

static constexpr class FakeComponent {} fakeComponent;

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

	ComponentBase(FakeComponent):
		state(ComponentState::Fake)
	{
	}

	ComponentId getComponentId() const
	{
		return componentId;
	}

	ComponentState state = ComponentState::Changed;

private:
	ComponentId componentId = Globals::ComponentIdGenerator().acquire();
};
