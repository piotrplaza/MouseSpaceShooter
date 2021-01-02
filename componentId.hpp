#pragma once

#include <vector>

using ComponentId = unsigned long;

class ComponentIdGenerator
{
public:
	static ComponentIdGenerator& instance();

	ComponentId acquire();
	ComponentId current() const;

	void release(ComponentId id);

private:
	ComponentIdGenerator() = default;

	static ComponentIdGenerator instance_;

	ComponentId counter = 0;
	std::vector<ComponentId> releasedIds;
};
