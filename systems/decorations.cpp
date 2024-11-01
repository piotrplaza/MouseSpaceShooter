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
		for (auto& decoration: Globals::Components().staticDecorations())
			decoration.step();

		for (auto& decoration: Globals::Components().decorations())
			decoration.step();

		updateDynamicBuffers();
	}

	void Decorations::updateStaticBuffers()
	{
		Tools::UpdateStaticBuffers(Globals::Components().staticDecorations(), loadedStaticDecorations);
		loadedStaticDecorations = Globals::Components().staticDecorations().size();
	}

	void Decorations::updateDynamicBuffers()
	{
		Tools::UpdateDynamicBuffers(Globals::Components().decorations());
	}
}
