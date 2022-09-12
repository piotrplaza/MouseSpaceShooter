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
		for (auto& decoration : Globals::Components().decorations())
			decoration.step();

		for (auto& [id, decoration] : Globals::Components().dynamicDecorations())
			decoration.step();

		updateDynamicBuffers();
	}

	void Decorations::updateStaticBuffers()
	{
		Tools::UpdateStaticBuffers(Globals::Components().decorations(), loadedStaticDecorations);
		loadedStaticDecorations = Globals::Components().decorations().size();
	}

	void Decorations::updateDynamicBuffers()
	{
		Tools::UpdateDynamicBuffers(Globals::Components().dynamicDecorations());
	}
}
