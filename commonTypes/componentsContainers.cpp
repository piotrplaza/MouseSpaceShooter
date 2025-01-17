#include "componentsContainers.hpp"

void ComponentsBase::CleanupAll()
{
	for (auto instance : StaticComponentsBase::instances)
		instance->teardown();
	for (auto instance : DynamicComponentsBase::instances)
		instance->teardown();

	for (auto instance : StaticComponentsBase::instances)
		instance->clear();
	for (auto instance : DynamicComponentsBase::instances)
		instance->clear();
}

void StaticComponentsBase::CleanupAll()
{
	for (auto instance : instances)
		instance->teardown();
	for (auto instance : instances)
		instance->clear();
}

void DynamicComponentsBase::CleanupAll()
{
	for (auto instance : instances)
		instance->teardown();
	for (auto instance : instances)
		instance->clear();
}

void DynamicComponentsBase::CleanupAllEnding()
{
	for (auto instance : instances)
		instance->cleanupEnding();
}

void DynamicComponentsBase::OutdateAll()
{
	for (auto instance : instances)
		instance->teardownReset();
	for (auto instance : instances)
		instance->markOutdated();
}
