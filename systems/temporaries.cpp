#include "temporaries.hpp"

#include <components/missile.hpp>

#include <globals/components.hpp>

#include <tools/buffersHelpers.hpp>

namespace Systems
{
	Temporaries::Temporaries() = default;

	void Temporaries::step()
	{
		for (auto& missile: Globals::Components().missiles())
			missile.step();

		updateDynamicBuffers();
	}

	void Temporaries::updateDynamicBuffers()
	{
		Tools::UpdateDynamicBuffers(Globals::Components().missiles());
	}
}
