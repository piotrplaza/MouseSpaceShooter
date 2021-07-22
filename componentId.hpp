#pragma once

#include <utility>

#include "idGenerator.hpp"

using ComponentId = unsigned long;
using ComponentIdGenerator = IdGenerator<ComponentId, 1>;

template <typename Container>
typename Container::mapped_type& EmplaceIdComponent(Container& container, typename Container::mapped_type&& component)
{
	const auto componentId = component.getComponentId();
	return container.emplace(componentId, std::forward<typename Container::mapped_type>(component)).first->second;
}
