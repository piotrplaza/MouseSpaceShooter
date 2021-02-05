#pragma once

#include <utility>

#include "idGenerator.hpp"

using ComponentId = unsigned long;
using ComponentIdGenerator = IdGenerator<ComponentId, 1>;

template <typename Container, typename ...ComponentArgs>
typename Container::mapped_type& EmplaceIdComponent(Container& container, ComponentArgs&&... componentArgs)
{
	const auto componentId = ComponentIdGenerator::instance().current();
	return container.emplace(componentId, Container::mapped_type(std::forward<ComponentArgs>(componentArgs)...)).first->second;
}
