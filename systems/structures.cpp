#include "structures.hpp"

#include <components/wall.hpp>
#include <components/grapple.hpp>

#include <globals/components.hpp>

#include <tools/buffersHelpers.hpp>

namespace Systems
{
	Structures::Structures() = default;

	void Structures::postInit()
	{
		updateStaticBuffers();
	}

	void Structures::step()
	{
		for (const auto& wall : Globals::Components().walls())
			if (wall.step)
				wall.step();

		for (const auto& [id, wall] : Globals::Components().dynamicWalls())
			if (wall.step)
				wall.step();

		for (auto& [id, grapple] : Globals::Components().grapples())
			if (grapple.step)
				grapple.step();

		updateDynamicBuffers();
	}

	void Structures::updateStaticBuffers()
	{
		Tools::UpdateStaticBuffers(Globals::Components().walls(), loadedStaticWalls);
		loadedStaticWalls = Globals::Components().walls().size();
	}

	void Structures::updateDynamicBuffers()
	{
		Tools::UpdateDynamicBuffers(Globals::Components().dynamicWalls());
		Tools::UpdateDynamicBuffers(Globals::Components().grapples());
	}
}
