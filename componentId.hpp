#pragma once

#include <utility>

#include "idGenerator.hpp"

using ComponentId = unsigned long;
using ComponentIdGenerator = IdGenerator<ComponentId>;

template <typename Component, typename ...Args>
std::pair<const ComponentId, Component> CreateIdComponent(Args&&... args)
{
	return { ComponentIdGenerator::instance().acquire(), Component(std::forward<Args>(args)...) };
}
