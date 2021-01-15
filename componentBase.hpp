#pragma once

#include <componentId.hpp>

enum class ComponentState {Ongoing, Changed, Outdated};

struct ComponentBase
{
	const ComponentId componentId = ComponentIdGenerator::instance().acquire();
	ComponentState state = ComponentState::Changed;
};
