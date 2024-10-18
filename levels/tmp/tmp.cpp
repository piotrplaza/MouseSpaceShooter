#include "tmp.hpp"

#include <components/graphicsSettings.hpp>
#include <components/decoration.hpp>
#include <components/texture.hpp>
#include <components/screenInfo.hpp>
#include <components/physics.hpp>
#include <components/keyboard.hpp>
#include <globals/components.hpp>

#include <ogl/shaders/textured.hpp>
#include <globals/shaders.hpp>

#include <tools/Shapes2D.hpp>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#define TEST 2

namespace Levels
{
	class Tmp::Impl
	{
	public:
		void setup()
		{
			Globals::Components().graphicsSettings().backgroundColorF = glm::vec4(0.0f, 0.1f, 0.0f, 1.0f);
			auto& texture = Globals::Components().dynamicTextures().emplace();
			auto& decoration = Globals::Components().dynamicDecorations().emplace();
#if TEST == 2
			texture.source = "textures/rose.png";
			decoration.vertices = Tools::Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 1.0f, 1.0f }, glm::quarter_pi<float>());
			decoration.resolutionMode = { ResolutionMode::Resolution::H405, ResolutionMode::Scaling::Nearest };
			decoration.texture = CM::DynamicTexture(&texture);
			decoration.renderingSetupF = [&](auto) {
				bool prevBlend = glProxyIsBlendEnabled();
				glProxySetBlend(false);
				return [prevBlend]() mutable {
					glProxySetBlend(prevBlend);
				};
			};
#endif
		}

		void step()
		{
#if TEST == 0
			const auto& screenInfo = Globals::Components().screenInfo();
			const auto pos = glm::linearRand(-screenInfo.getNormalizedWindowSize(), screenInfo.getNormalizedWindowSize()) * 9.0f;
			auto& decoration = Globals::Components().dynamicDecorations().emplace(Tools::Shapes2D::CreateVerticesOfCircle(pos, 1.0f, 20));
			decoration.subsequence.emplace_back(Tools::Shapes2D::CreateVerticesOfCircle(pos, 0.2f, 20));
			decoration.subsequence.back().colorF = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			decoration.state = ComponentState::LastShot;
#elif TEST == 1
			auto& decoration = Globals::Components().dynamicDecorations().last(); 
			decoration.vertices = Tools::Shapes2D::CreateVerticesOfLightning({ -5.0f, 0.0f }, { 5.0f, 0.0f }, 10);
			decoration.renderF = []() { return Globals::Components().keyboard().pressing[' ']; };
			decoration.state = ComponentState::Changed;
			decoration.drawMode = GL_LINE_STRIP;
#endif
		}
	};

	Tmp::Tmp():
		impl(std::make_unique<Impl>())
	{
		impl->setup();
	}

	Tmp::~Tmp() = default;

	void Tmp::step()
	{
		impl->step();
	}
}
