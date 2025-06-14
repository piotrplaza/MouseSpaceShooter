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
		for (auto& wall : Globals::Components().staticWalls())
			wall.step();

		for (auto& wall : Globals::Components().walls())
			wall.step();

		for (auto& grapple : Globals::Components().staticGrapples())
			grapple.step();

		for (auto& grapple : Globals::Components().grapples())
			grapple.step();

		for (auto& polyline : Globals::Components().staticPolylines())
			polyline.step();

		for (auto& polyline : Globals::Components().polylines())
			polyline.step();

		updateDynamicBuffers();
	}

	void Structures::updateStaticBuffers()
	{
		Tools::UpdateStaticBuffers(Globals::Components().staticWalls(), loadedStaticWalls);
		loadedStaticWalls = Globals::Components().staticWalls().size();

		Tools::UpdateStaticBuffers(Globals::Components().staticGrapples(), loadedStaticGrapples);
		loadedStaticGrapples = Globals::Components().staticGrapples().size();

		Tools::UpdateStaticBuffers(Globals::Components().staticPolylines(), loadedStaticPolylines);
		loadedStaticPolylines = Globals::Components().staticPolylines().size();
	}

	void Structures::updateDynamicBuffers()
	{
		Tools::UpdateDynamicBuffers(Globals::Components().walls());
		Tools::UpdateDynamicBuffers(Globals::Components().grapples());
		Tools::UpdateDynamicBuffers(Globals::Components().polylines());
	}
}
