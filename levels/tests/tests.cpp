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
			const auto pos = glm::linearRand(-screenInfo.getNormalizedFramebufferRes(), screenInfo.getNormalizedFramebufferRes()) * 9.0f;
			auto& decoration = Globals::Components().decorations().emplace(Tools::Shapes2D::CreateVerticesOfCircle(pos, 1.0f, 20));
			decoration.subsequence.emplace_back(Tools::Shapes2D::CreateVerticesOfCircle(pos, 0.2f, 20));
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
			auto& decoration = Globals::Components().decorations().emplace();
			auto& texture = Globals::Components().textures().emplace();
			texture.source = "textures/rose.png";
			decoration.vertices = Tools::Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 1.0f, 1.0f }, glm::quarter_pi<float>());
			decoration.resolutionMode = { ResolutionMode::Resolution::H405, ResolutionMode::Scaling::Nearest };
			decoration.texture = CM::Texture(texture);
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
			//Globals::Components().textures().emplace("textures/rose.png");
			Globals::Components().decorations().emplace(Tools::Shapes2D::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 1.0f, 1.0f }), CM::DummyTexture()/*CM::Texture(&Globals::Components().textures().last())*/);
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

	auto activeTest = std::make_unique<Music>();
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
