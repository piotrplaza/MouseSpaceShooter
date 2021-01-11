#pragma once

enum class ComponentState {Ongoing, Changed, Outdated};

struct ComponentBase
{
	ComponentState state = ComponentState::Changed;
};
