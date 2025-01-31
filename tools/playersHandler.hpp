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

	class PlayersHandler
	{
	public:
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

		void initPlayers(const std::array<CM::Texture, 4>& planeTexturesForPlayers, const std::array<CM::AnimatedTexture, 4>& flameAnimatedTexturesForPlayers, bool gamepadForPlayer1,
			std::function<glm::vec3(unsigned playerId, unsigned numOfPlayers)> initLocF, bool centerToFront = false, std::optional<CM::SoundBuffer> thrustSoundBuffer = std::nullopt,
			std::optional<CM::SoundBuffer> grappleSoundBuffer = std::nullopt);
		void setCamera(CameraParams cameraParams);

		void gamepadsAutodetectionStep(std::function<glm::vec3(unsigned player)> initLocF);
		void controlStep(std::function<void(unsigned playerHandlerId, bool fire)> fireF = nullptr);

		const std::vector<Tools::PlayerHandler>& getPlayersHandlers() const;
		std::vector<Tools::PlayerHandler>& accessPlayersHandlers();

		const std::vector<const Tools::PlayerHandler*> getActivePlayersHandlers() const;
		std::vector<Tools::PlayerHandler*> accessActivePlayersHandlers();

	private:
		std::vector<Tools::PlayerHandler> playersHandlers;
		std::array<CM::Texture, 4> planeTextures;
		std::array<CM::AnimatedTexture, 4> flameAnimatedTexturesForPlayers;
		bool gamepadForPlayer1 = false;
		std::optional<CM::SoundBuffer> thrustSoundBuffer;
		std::optional<CM::SoundBuffer> grappleSoundBuffer;
	};
}
