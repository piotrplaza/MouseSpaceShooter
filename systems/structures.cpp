#include "structures.hpp"

#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/polyline.hpp>

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
		for (auto& wall : Globals::Components().walls())
			wall.step();

		for (auto& wall : Globals::Components().dynamicWalls())
			wall.step();

		for (auto& grapple : Globals::Components().grapples())
			grapple.step();

		for (auto& polyline : Globals::Components().polylines())
			polyline.step();

		updateDynamicBuffers();
	}

	void Structures::updateStaticBuffers()
	{
		Tools::UpdateStaticBuffers(Globals::Components().walls(), loadedStaticWalls);
		loadedStaticWalls = Globals::Components().walls().size();

		Tools::UpdateStaticBuffers(Globals::Components().polylines(), loadedStaticPolylines);
		loadedStaticPolylines = Globals::Components().polylines().size();
	}

	void Structures::updateDynamicBuffers()
	{
		Tools::UpdateDynamicBuffers(Globals::Components().dynamicWalls());
		Tools::UpdateDynamicBuffers(Globals::Components().grapples());
	}
}
