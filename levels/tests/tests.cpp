#include "tests.hpp"

#include <components/graphicsSettings.hpp>
#include <components/decoration.hpp>
#include <components/texture.hpp>
#include <components/systemInfo.hpp>
#include <components/physics.hpp>
#include <components/keyboard.hpp>
#include <globals/components.hpp>

#include <ogl/shaders/textured.hpp>
#include <globals/shaders.hpp>

#include <tools/Shapes2D.hpp>
#include <ogl/oglHelpers.hpp>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include <iostream>

namespace Tests
{
	struct TestBase
	{
		virtual void setup() {}
		virtual void teardown() {}
		virtual void step() {}
	};

	struct SubsequenceLastShot : public TestBase
	{
		void step() override
		{
			const auto& screenInfo = Globals::Components().systemInfo().screen;
			const auto pos = glm::linearRand(-screenInfo.getNormalizedWindowSize(), screenInfo.getNormalizedWindowSize()) * 9.0f;
			auto& decoration = Globals::Components().dynamicDecorations().emplace(Tools::Shapes2D::CreateVerticesOfCircle(pos, 1.0f, 20));
			decoration.subsequence.emplace_back(Tools::Shapes2D::CreateVerticesOfCircle(pos, 0.2f, 20));
			decoration.subsequence.back().colorF = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			decoration.state = ComponentState::LastShot;
		}
	};

	struct NoRenderAtStart : public TestBase
	{
		void setup() override
		{
			Globals::Components().dynamicDecorations().emplace();
		}

		void step() override
		{
			auto& decoration = Globals::Components().dynamicDecorations().last();
			decoration.vertices = Tools::Shapes2D::CreateVerticesOfLightning({ -5.0f, 0.0f }, { 5.0f, 0.0f }, 10);
			decoration.renderF = []() { return Globals::Components().keyboard().pressing[' ']; };
			decoration.state = ComponentState::Changed;
			decoration.drawMode = GL_LINE_STRIP;
		}
	};

	struct CustomResNoBlending : public TestBase
	{
		void setup() override
		{
			auto& decoration = Globals::Components().dynamicDecorations().emplace();
			auto& texture = Globals::Components().dynamicTextures().emplace();
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
		}
	};

	struct RepeatedTextureLoading : public TestBase
	{
		void setup() override
		{
			//Globals::Components().dynamicTextures().emplace("textures/rose.png");
			Globals::Components().dynamicDecorations().emplace(Tools::Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 1.0f, 1.0f }), CM::DummyTexture()/*CM::DynamicTexture(&Globals::Components().dynamicTextures().last())*/);
		}

		void step() override
		{
			Globals::Components().dynamicTextures().emplace("textures/rose.png");
			Globals::Components().dynamicTextures().last().state = ComponentState::LastShot;
			Globals::Components().dynamicDecorations().last().texture = CM::DynamicTexture(&Globals::Components().dynamicTextures().last());
			//std::cout << "Textures count: " << Globals::Components().dynamicTextures().size() << std::endl;
		}
	};

	auto activeTest = std::make_unique<RepeatedTextureLoading>();
}

namespace Levels
{
	Tests::Tests()
	{
		Globals::Components().graphicsSettings().backgroundColorF = glm::vec4(0.0f, 0.1f, 0.0f, 1.0f);
		::Tests::activeTest->setup();
	}

	Tests::~Tests()
	{
		::Tests::activeTest->teardown();
	}

	void Tests::step()
	{
		::Tests::activeTest->step();
	}
}
