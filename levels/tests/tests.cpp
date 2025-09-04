#include "tests.hpp"

#include <components/graphicsSettings.hpp>
#include <components/decoration.hpp>
#include <components/texture.hpp>
#include <components/systemInfo.hpp>
#include <components/physics.hpp>
#include <components/keyboard.hpp>
#include <components/SoundBuffer.hpp>
#include <components/sound.hpp>
#include <components/music.hpp>
#include <components/gamepad.hpp>
#include <components/renderTexture.hpp>
#include <globals/components.hpp>

#include <ogl/shaders/textured.hpp>
#include <globals/shaders.hpp>

#include <tools/Shapes2D.hpp>
#include <tools/particleSystemHelpers.hpp>
#include <tools/gameHelpers.hpp>
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

	struct BasicBox : public TestBase
	{
		void setup() override
		{
			Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle());
		}
	};

	struct BasicLayers : public TestBase
	{
		void setup() override
		{
			Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle());
			Globals::Components().staticDecorations().last().colorF = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			Globals::Components().staticDecorations().last().renderLayer = RenderLayer::Midground;

			Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle({ 0.2f, 0.2f }));
			Globals::Components().staticDecorations().last().colorF = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
			Globals::Components().staticDecorations().last().renderLayer = RenderLayer::Background;
		}
	};

	struct MultiShaderLayers : public TestBase
	{
		void setup() override
		{
			Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle(glm::vec2(0.0f), glm::vec2(2.0f)));
			Globals::Components().staticDecorations().last().colorF = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			Globals::Components().staticDecorations().last().renderLayer = RenderLayer::Midground;

			Tools::CreateJuliaBackground(Tools::JuliaParams{}).renderLayer = RenderLayer::Background;
		}
	};

	struct SubsequenceLastShot : public TestBase
	{
		void step() override
		{
			const auto& screenInfo = Globals::Components().systemInfo().screen;
			const auto pos = glm::linearRand(-screenInfo.getNormalizedFramebufferRes(), screenInfo.getNormalizedFramebufferRes()) * 9.0f;
			auto& decoration = Globals::Components().decorations().emplace(Tools::Shapes2D::CreatePositionsOfDisc(pos, 1.0f, 20));
			decoration.subsequence.emplace_back(Tools::Shapes2D::CreatePositionsOfDisc(pos, 0.2f, 20));
			decoration.subsequence.back().colorF = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			decoration.state = ComponentState::LastShot;
		}
	};

	struct NoRenderAtStart : public TestBase
	{
		void setup() override
		{
			Globals::Components().decorations().emplace();
		}

		void step() override
		{
			auto& decoration = Globals::Components().decorations().last();
			decoration.positions = Tools::Shapes2D::CreatePositionsOfLightning({ -5.0f, 0.0f }, { 5.0f, 0.0f }, 10);
			decoration.renderF = []() { return Globals::Components().keyboard().pressing[' ']; };
			decoration.state = ComponentState::Changed;
			decoration.drawMode = GL_LINE_STRIP;
		}
	};

	struct CustomResNoBlending : public TestBase
	{
		void setup() override
		{
			auto& decoration = Globals::Components().decorations().emplace();
			auto& texture = Globals::Components().textures().emplace();
			texture.source = "textures/rose.png";
			decoration.positions = Tools::Shapes2D::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 1.0f, 1.0f }, glm::quarter_pi<float>());
			decoration.targetTextures = { Globals::Components().standardRenderTexture({ StandardRenderMode::Resolution::H405, StandardRenderMode::Scaling::Nearest, StandardRenderMode::mainBlending }) };
			decoration.texture = CM::Texture(texture);
			decoration.renderingSetupF = [&](auto&) {
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
			//Globals::Components().textures().emplace("textures/rose.png");
			Globals::Components().decorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 1.0f, 1.0f }), CM::DummyTexture()/*CM::Texture(&Globals::Components().textures().last())*/);
		}

		void step() override
		{
			Globals::Components().textures().emplace("textures/rose.png");
			Globals::Components().textures().last().state = ComponentState::LastShot;
			Globals::Components().decorations().last().texture = CM::Texture(Globals::Components().textures().last());
			//std::cout << "Textures count: " << Globals::Components().textures().size() << std::endl;
		}
	};

	struct StoppedSoundCracking : public TestBase
	{
		void setup() override
		{
			Globals::Components().soundsBuffers().emplace("audio/Ghosthack Synth - Choatic_C.wav");
			Globals::Components().sounds().emplace(Globals::Components().soundsBuffers().last()).setLooping(true);
		}

		void step() override
		{
			if (Globals::Components().keyboard().pressed[' '])
			{
				//Globals::Components().sounds().last().setVolume(1.0f);
				Globals::Components().sounds().last().play();
			}
			else if (Globals::Components().keyboard().released[' '])
			{
				//Globals::Components().sounds().last().setVolume(0.0f);
				Globals::Components().sounds().last().pause();
			}
		}
	};

	struct Music : public TestBase
	{
		void setup() override
		{
			Globals::Components().musics().emplace("audio/DamageOn 5.ogg").play();
		}

		void step() override
		{
			if (Globals::Components().keyboard().pressed[' '])
			{
				Globals::Components().musics().last().setVolume(0.0f);
			}
			else if (Globals::Components().keyboard().released[' '])
			{
				Globals::Components().musics().last().setVolume(1.0f);
			}
		}

		float volume = 1.0f;
	};

	struct ParticleSystemsSwitching : public TestBase
	{
		void setup() override
		{
			Tools::CreateParticleSystem({});
		}

		void step() override
		{
			if (Globals::Components().keyboard().pressed[' '])
			{
				Tools::CreateSparking({});
				//const auto pos = glm::vec3(0.0f, 1.0f, 0.0f);
				//Tools::CreateParticleSystem(Tools::ParticleSystemParams{}.position(pos).prevPosition(pos).velocityFactor(10.0f));
			}
		}
	};

	struct Gamepad : public TestBase
	{
		void step() override
		{
			const auto& gamepads = Globals::Components().gamepads();
			for (const auto& gamepad : gamepads)
			{
				if (gamepad.isEnabled())
				{
					std::cout << gamepad.getComponentId() << ": " << gamepad.lStick.x << ", " << gamepad.lStick.y << std::endl;
				}
			}
		}
	};

	struct CustomRenderTexture : public TestBase
	{
		void setup() override
		{
			auto& renderTexture = Globals::Components().renderTextures().emplace(glm::vec2(100));
			renderTexture.borderColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
			Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle({ 0.0f, 0.0f }, glm::vec2(5.0f)));
			Globals::Components().staticDecorations().last().targetTextures = { renderTexture };
			Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle({ 0.0f, 0.0f }, glm::vec2(10.0f)), renderTexture, Tools::Shapes2D::CreateTexCoordOfRectangle());
		}
	};

	struct CustomRenderTextureTwoTargets : public TestBase
	{
		void setup() override
		{
			auto& renderTexture1 = Globals::Components().renderTextures().emplace(glm::vec2(100));
			renderTexture1.borderColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
			auto& renderTexture2 = Globals::Components().renderTextures().emplace(glm::vec2(20));
			renderTexture2.borderColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

			Globals::Components().staticDecorations().emplace(std::vector<glm::vec3>{ {-1.0f, -1.0f, 0.0f}, { 1.0f, 1.0f, 0.0f} });
			Globals::Components().staticDecorations().last().targetTextures = { renderTexture1, renderTexture2 };
			Globals::Components().staticDecorations().last().drawMode = GL_LINES;

			Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle({ -8.0f, 0.0f }, glm::vec2(6.0f)), renderTexture1, Tools::Shapes2D::CreateTexCoordOfRectangle());
			Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle({ 8.0f, 0.0f }, glm::vec2(6.0f)), renderTexture2, Tools::Shapes2D::CreateTexCoordOfRectangle());
		}
	};

	auto activeTest = std::make_unique<CustomRenderTextureTwoTargets>();
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
