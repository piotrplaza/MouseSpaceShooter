#include "noise.hpp"

#include <components/graphicsSettings.hpp>
#include <components/texture.hpp>

#include <components/decoration.hpp>
#include <components/physics.hpp>

#include <ogl/shaders/noise.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <tools/Shapes2D.hpp>

namespace Levels
{
	class Noise::Impl
	{
	public:
		void setup()
		{
			auto& screen = Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 1.0f, 1.0f }));
			screen.customShadersProgram = &Globals::Shaders().noise();

			Globals::Shaders().noise().frameSetupF = [&](auto& program) {
				program.color(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
				program.separateNoises(true);
			};
		}
	};

	Noise::Noise():
		impl(std::make_unique<Impl>())
	{
		impl->setup();
	}
}
