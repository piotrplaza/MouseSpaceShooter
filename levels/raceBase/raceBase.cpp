#include "raceBase.hpp"

#include <components/graphicsSettings.hpp>
#include <components/mainFramebufferRenderer.hpp>
#include <components/camera.hpp>
#include <components/plane.hpp>
#include <components/mouse.hpp>
#include <components/gamepad.hpp>
#include <components/soundBuffer.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <ogl/renderingHelpers.hpp>
#include <ogl/shaders/textured.hpp>

#include <tools/gameHelpers.hpp>
#include <tools/playersHandler.hpp>

namespace Levels
{
	class RaceBase::Impl
	{
	public:
		void setGraphicsSettings() const
		{
			Globals::Components().graphicsSettings().clearColor = { 0.7f, 0.8f, 0.9f, 1.0f };
			Globals::Components().mainFramebufferRenderer().renderer = Tools::StandardFullscreenRenderer(Globals::Shaders().textured());
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().textures();

			planeTextures[0] = textures.size();
			textures.emplace("textures/plane 1.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.6f, 1.8f);
			textures.last().minFilter = GL_LINEAR;

			planeTextures[1] = textures.size();
			textures.emplace("textures/alien ship 1.png");
			textures.last().translate = glm::vec2(-0.2f, 0.0f);
			textures.last().scale = glm::vec2(1.9f);
			textures.last().minFilter = GL_LINEAR;

			planeTextures[2] = textures.size();
			textures.emplace("textures/plane 2.png");
			textures.last().translate = glm::vec2(0.4f, 0.0f);
			textures.last().scale = glm::vec2(1.8f, 1.8f);
			textures.last().minFilter = GL_LINEAR;

			planeTextures[3] = textures.size();
			textures.emplace("textures/alien ship 2.png");
			textures.last().translate = glm::vec2(0.0f, 0.0f);
			textures.last().scale = glm::vec2(1.45f, 1.4f);
			textures.last().minFilter = GL_LINEAR;

			flameAnimationTexture = textures.size();
			textures.emplace("textures/flame animation 1.jpg");
			textures.last().minFilter = GL_LINEAR;
		}

		void loadAudio()
		{
			auto& soundsBuffers = Globals::Components().soundsBuffers();

			thrustSoundBuffer = soundsBuffers.emplace("audio/thrust.wav", 0.2f).getComponentId();
			grappleSoundBuffer = soundsBuffers.emplace("audio/Ghosthack Synth - Choatic_C.wav").getComponentId();
		}

		void setAnimations()
		{
			for (unsigned& flameAnimatedTextureForPlayer : flameAnimatedTextureForPlayers)
			{
				flameAnimatedTextureForPlayer = Globals::Components().animatedTextures().size();
				Globals::Components().animatedTextures().add({ flameAnimationTexture, { 500, 498 }, { 8, 4 }, { 3, 0 }, 442, 374, { 55, 122 }, 0.02f, 32, 0,
					AnimationDirection::Backward, AnimationPolicy::Repeat, TextureLayout::Horizontal });
				Globals::Components().animatedTextures().last().start(true);
			}
		}

		void setCamera()
		{
			playersHandler.setCamera(Tools::PlayersHandler::CameraParams().projectionHSizeMin([]() { return 30.0f; }).scalingFactor(0.7f));
		}

		void step()
		{
			playersHandler.controlStep([this](unsigned playerHandlerId, bool fire) {
				if (playersHandler.getActivePlayersHandlers().size() == 1 && Globals::Components().planes()[playersHandler.getActivePlayersHandlers().front()->playerId].controls.startPressed)
					reset();
			});
		}

		void reset()
		{
			playersHandler.initPlayers(planeTextures, flameAnimatedTextureForPlayers, false,
				[this](unsigned player, auto) {
					return glm::vec3(0.0f);
				}, thrustSoundBuffer, grappleSoundBuffer);
		}

	private:
		std::array<unsigned, 4> planeTextures{ 0 };
		ComponentId flameAnimationTexture = 0;

		std::array<unsigned, 4> flameAnimatedTextureForPlayers{ 0 };

		ComponentId thrustSoundBuffer = 0;
		ComponentId grappleSoundBuffer = 0;

		Tools::PlayersHandler playersHandler;
	};

	RaceBase::RaceBase() :
		impl(std::make_unique<Impl>())
	{
		impl->setGraphicsSettings();
		impl->loadTextures();
		impl->loadAudio();
		impl->setAnimations();
		impl->setCamera();

		impl->reset();
	}

	RaceBase::~RaceBase() = default;

	void RaceBase::step()
	{
		impl->step();
	}
}
