#include "temporaries.hpp"

#include <components/missile.hpp>
#include <components/shockwave.hpp>

#include <globals/components.hpp>

#include <tools/buffersHelpers.hpp>

namespace Systems
{
	Temporaries::Temporaries() = default;

	void Temporaries::step()
	{
		for (auto& missile: Globals::Components().missiles())
			missile.step();
		for (auto& shockwave : Globals::Components().shockwaves())
			shockwave.step();

		updateDynamicBuffers();
	}

	void Temporaries::updateDynamicBuffers()
	{
		Tools::ProcessDynamicComponents(Globals::Components().missiles());
	}
}
