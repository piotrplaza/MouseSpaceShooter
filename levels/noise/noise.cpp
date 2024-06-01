#include "noise.hpp"

#include <components/graphicsSettings.hpp>
#include <components/texture.hpp>

#include <components/decoration.hpp>
#include <components/physics.hpp>

#include <ogl/shaders/noise.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <tools/shapes2D.hpp>

namespace Levels
{
	class Noise::Impl
	{
	public:
		void setup()
		{
			auto& screen = Globals::Components().staticDecorations().emplace(Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 1.0f, 1.0f }));
			auto& duration = Globals::Components().physics().simulationDuration;
			screen.customShadersProgram = Globals::Shaders().noise().getProgramId();

			Globals::Shaders().noise().frameSetupF = [&](auto& program) {
				program.color(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
				program.separateNoises(true);
				program.time(duration);
			};
		}
	};

	Noise::Noise():
		impl(std::make_unique<Impl>())
	{
		impl->setup();
	}
}
