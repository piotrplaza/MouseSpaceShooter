#pragma once

#include <utility>

#include "idGenerator.hpp"

using ComponentId = unsigned long;
using ComponentIdGenerator = IdGenerator<ComponentId, 1>;

template <typename Component, typename ...Args>
std::pair<const ComponentId, Component> CreateIdComponent(Args&&... args)
{
	return { ComponentIdGenerator::instance().current(), Component(std::forward<Args>(args)...) };
}
