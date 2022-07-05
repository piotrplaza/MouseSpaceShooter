#include "temporaries.hpp"

#include <components/missile.hpp>

#include <globals/components.hpp>

#include <ogl/buffersHelpers.hpp>

namespace Systems
{
	Temporaries::Temporaries() = default;

	void Temporaries::step()
	{
		for (auto& [id, missile] : Globals::Components().missiles())
			if (missile.step)
				missile.step();

		updateDynamicBuffers();
	}

	void Temporaries::updateDynamicBuffers()
	{
		Tools::UpdateDynamicBuffers(Globals::Components().missiles());
	}
}
