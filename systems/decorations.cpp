#include "decorations.hpp"

#include <components/decoration.hpp>

#include <globals/components.hpp>

#include <tools/buffersHelpers.hpp>

namespace Systems
{
	Decorations::Decorations() = default;

	void Decorations::postInit()
	{
		updateStaticBuffers();
	}

	void Decorations::step()
	{
		for (const auto& decoration : Globals::Components().decorations())
			if (decoration.step)
				decoration.step();

		for (const auto& [id, decoration] : Globals::Components().dynamicDecorations())
			if (decoration.step)
				decoration.step();

		updateDynamicBuffers();
	}

	void Decorations::updateStaticBuffers()
	{
		Tools::UpdateStaticBuffers(Globals::Components().decorations());
	}

	void Decorations::updateDynamicBuffers()
	{
		Tools::UpdateDynamicBuffers(Globals::Components().dynamicDecorations());
	}
}
