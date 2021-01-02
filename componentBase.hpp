#pragma once

#include "componentId.hpp"

enum class ComponentState {Current, Changed, Deleted};

struct ComponentBase
{
	static void ReleaseId(ComponentId id)
	{
		ComponentIdGenerator::instance().release(id);
	}

	ComponentBase();

	const ComponentId id;
	ComponentState state = ComponentState::Changed;
};
