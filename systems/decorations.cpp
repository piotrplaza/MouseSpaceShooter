#include "decorations.hpp"

#include <components/decoration.hpp>
#include <components/particles.hpp>

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

		for (auto& particles : Globals::Components().staticParticles())
			particles.step();

		for (auto& particles : Globals::Components().particles())
			particles.step();

		updateDynamicBuffers();
	}

	void Decorations::updateStaticBuffers()
	{
		Tools::ProcessStaticComponents(Globals::Components().staticDecorations(), loadedStaticDecorations);
		loadedStaticDecorations = Globals::Components().staticDecorations().size();
		Tools::ProcessStaticComponents(Globals::Components().staticParticles(), loadedStaticParticles);
		loadedStaticParticles = Globals::Components().staticParticles().size();
	}

	void Decorations::updateDynamicBuffers()
	{
		Tools::ProcessDynamicComponents(Globals::Components().decorations());
		Tools::ProcessDynamicComponents(Globals::Components().particles());
	}
}
