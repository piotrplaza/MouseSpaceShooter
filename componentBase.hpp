#pragma once

enum class ComponentState {Current, Changed, Deleted};

struct ComponentBase
{
	ComponentState state = ComponentState::Changed;
};
