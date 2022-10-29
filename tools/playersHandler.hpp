#pragma once

#include <commonTypes/componentId.hpp>

#include "gameHelpers.hpp"

#include <glm/glm.hpp>

#include <array>
#include <vector>
#include <functional>
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
	};

	class PlayersHandler
	{
	public:
		struct CameraParams
		{
			CameraParams& projectionHSizeMin(std::function<float()> value)
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

			CameraParams& additionalActors(std::function<glm::vec2()> value)
			{
				additionalActors_.push_back(std::move(value));
				return *this;
			}

			std::function<float()> projectionHSizeMin_;
			float scalingFactor_ = 0.6f;
			float velocityFactor_ = 0.2f;
			float transitionFactor_ = 10.0f;
			std::vector<std::function<glm::vec2()>> additionalActors_;
		};

		void initPlayers(ComponentId rocketPlaneTexture, const std::array<unsigned, 4>& flameAnimatedTextureForPlayers, bool gamepadForPlayer1,
			std::function<glm::vec2(unsigned player, unsigned numOfPlayers)> initPosF, std::optional<ComponentId> thrustSoundBuffer = std::nullopt,
			std::optional<ComponentId> grappleSoundBuffer = std::nullopt);
		void setCamera(CameraParams cameraParams) const;

		void autodetectionStep(std::function<glm::vec2(unsigned player)> initPosF);
		void controlStep(std::function<void(unsigned playerHandlerId, bool fire)> fireF = nullptr);

		const std::vector<Tools::PlayerHandler>& getPlayersHandlers() const;
		std::vector<Tools::PlayerHandler>& accessPlayersHandlers();

		const std::vector<const Tools::PlayerHandler*> getActivePlayersHandlers() const;
		std::vector<Tools::PlayerHandler*> accessActivePlayersHandlers();

	private:
		std::optional<ComponentId> createSound(std::optional<ComponentId> soundBuffer, ComponentId planeId) const;

		std::vector<Tools::PlayerHandler> playersHandlers;
		ComponentId rocketPlaneTexture = 0;
		std::array<unsigned, 4> flameAnimatedTextureForPlayers{ 0 };
		bool gamepadForPlayer1 = false;
		std::optional<ComponentId> thrustSoundBuffer;
		std::optional<ComponentId> grappleSoundBuffer;
	};
}
