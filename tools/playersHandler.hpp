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

		void initPlayers(const std::array<unsigned, 4>& planeTexturesForPlayers, const std::array<unsigned, 4>& flameAnimatedTexturesForPlayers, bool gamepadForPlayer1,
			std::function<glm::vec3(unsigned player, unsigned numOfPlayers)> initLocF, std::optional<ComponentId> thrustSoundBuffer = std::nullopt,
			std::optional<ComponentId> grappleSoundBuffer = std::nullopt);
		void setCamera(CameraParams cameraParams) const;

		void autodetectionStep(std::function<glm::vec3(unsigned player)> initLocF);
		void controlStep(std::function<void(unsigned playerHandlerId, bool fire)> fireF = nullptr);

		const std::vector<Tools::PlayerHandler>& getPlayersHandlers() const;
		std::vector<Tools::PlayerHandler>& accessPlayersHandlers();

		const std::vector<const Tools::PlayerHandler*> getActivePlayersHandlers() const;
		std::vector<Tools::PlayerHandler*> accessActivePlayersHandlers();

	private:
		std::vector<Tools::PlayerHandler> playersHandlers;
		std::array<unsigned, 4> planeTextures{ 0 };
		std::array<unsigned, 4> flameAnimatedTexturesForPlayers{ 0 };
		bool gamepadForPlayer1 = false;
		std::optional<ComponentId> thrustSoundBuffer;
		std::optional<ComponentId> grappleSoundBuffer;
	};
}
