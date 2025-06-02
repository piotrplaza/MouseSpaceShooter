#pragma once

#include <commonTypes/componentId.hpp>

#include "gameHelpers.hpp"

#include <commonTypes/fTypes.hpp>

#include <glm/glm.hpp>

#include <array>
#include <vector>
#include <optional>

namespace Tools
{
	class PlayersHandler
	{
	public:
		struct PlayerHandler
		{
			ComponentId playerId = 0;
			std::optional<ComponentId> gamepadId;
			float durationToLaunchMissile = 0.0f;
			std::optional<ComponentId> thrustSound;
			float thrustVolume = 0.0f;
			std::optional<ComponentId> grappleSound;
			float grappleVolume = 0.0f;
			std::optional<float> disabledTime;
		};

		struct InitPlayerParams
		{
			InitPlayerParams& planeTextures(const std::array<CM::Texture, 4>& textures)
			{
				planeTextures_ = textures;
				return *this;
			}

			InitPlayerParams& flameTextures(const std::array<CM::AnimatedTexture, 4>& textures)
			{
				flameAnimatedTextures_ = textures;
				return *this;
			}

			InitPlayerParams& gamepadForPlayer1(bool enabled)
			{
				gamepadForPlayer1_ = enabled;
				return *this;
			}

			InitPlayerParams& initLocationFunc(std::function<glm::vec3(unsigned, unsigned)> func)
			{
				initLocF_ = std::move(func);
				return *this;
			}

			InitPlayerParams& centerToFront(bool enabled)
			{
				centerToFront_ = enabled;
				return *this;
			}

			InitPlayerParams& thrustSound(CM::SoundBuffer buffer)
			{
				thrustSoundBuffer_ = std::move(buffer);
				return *this;
			}

			InitPlayerParams& grappleSound(CM::SoundBuffer buffer)
			{
				grappleSoundBuffer_ = std::move(buffer);
				return *this;
			}

			InitPlayerParams& soundAttenuation(float attenuation)
			{
				soundAttenuation_ = attenuation;
				return *this;
			}

			std::array<CM::Texture, 4> planeTextures_;
			std::array<CM::AnimatedTexture, 4> flameAnimatedTextures_;
			bool gamepadForPlayer1_ = false;
			std::function<glm::vec3(unsigned, unsigned)> initLocF_;
			bool centerToFront_ = false;
			std::optional<CM::SoundBuffer> thrustSoundBuffer_ = std::nullopt;
			std::optional<CM::SoundBuffer> grappleSoundBuffer_ = std::nullopt;
			float soundAttenuation_ = 1.0f;
		};

		struct CameraParams
		{
			CameraParams& projectionHSizeMin(FFloat value)
			{
				projectionHSizeMin_ = value;
				return *this;
			}

			CameraParams& scalingFactor(float value)
			{
				scalingFactor_ = value;
				return *this;
			}

			CameraParams& velocityFactor(float value)
			{
				velocityFactor_ = value;
				return *this;
			}

			CameraParams& transitionFactor(float value)
			{
				transitionFactor_ = value;
				return *this;
			}

			CameraParams& trackingTimeAfterDisabled(float value)
			{
				trackingTimeAfterDisabled_ = value;
				return *this;
			}

			CameraParams& additionalActors(FVec2 value)
			{
				additionalActors_.push_back(std::move(value));
				return *this;
			}

			FFloat projectionHSizeMin_;
			float scalingFactor_ = 0.6f;
			float velocityFactor_ = 0.1f;
			float transitionFactor_ = 10.0f;
			float trackingTimeAfterDisabled_ = 1.0f;
			std::vector<FVec2> additionalActors_;
		};

		~PlayersHandler();

		void initPlayers(InitPlayerParams params);
		void setCamera(CameraParams params);

		void gamepadsAutodetectionStep(std::function<glm::vec3(unsigned player)> initLocF);
		void controlStep(std::function<void(unsigned playerHandlerId, bool fire)> fireF = nullptr);

		const std::vector<PlayerHandler>& getPlayersHandlers() const;
		std::vector<PlayerHandler>& accessPlayersHandlers();

		const std::vector<const PlayerHandler*> getActivePlayersHandlers() const;
		std::vector<PlayerHandler*> accessActivePlayersHandlers();

	private:
		std::vector<PlayerHandler> playersHandlers;
		std::array<CM::Texture, 4> planeTextures;
		std::array<CM::AnimatedTexture, 4> flameAnimatedTexturesForPlayers;
		bool gamepadForPlayer1 = false;
		std::optional<CM::SoundBuffer> thrustSoundBuffer;
		std::optional<CM::SoundBuffer> grappleSoundBuffer;
		float soundAttenuation = 1.0f;
	};
}
