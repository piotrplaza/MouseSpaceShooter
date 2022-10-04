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
		float volume = 0.0f;
	};

	class PlayersHandler
	{
	public:
		void initPlayers(ComponentId rocketPlaneTexture, const std::array<unsigned, 4>& flameAnimatedTextureForPlayers, bool gamepadForPlayer1,
			std::function<glm::vec2(unsigned player, unsigned numOfPlayers)> initPosF, std::optional<ComponentId> thrustSoundBuffer = std::nullopt);
		void initMultiplayerCamera(std::function<float()> projectionHSizeMin, float scalingFactor = 0.6f, float velocityFactor = 0.2f, float transitionFactor = 10.0f) const;

		void autodetectionStep(std::function<glm::vec2(unsigned player)> initPosF);
		void controlStep(std::function<void(unsigned playerHandlerId, bool fire)> fireF = nullptr);

		const std::vector<Tools::PlayerHandler>& getPlayersHandlers() const;
		std::vector<Tools::PlayerHandler>& accessPlayersHandlers();

	private:
		std::optional<ComponentId> createThrustSound(ComponentId planeId) const;
		std::vector<Tools::PlayerHandler> playersHandlers;

		ComponentId rocketPlaneTexture = 0;
		std::array<unsigned, 4> flameAnimatedTextureForPlayers{ 0 };
		bool gamepadForPlayer1 = false;
		std::optional<ComponentId> thrustSoundBuffer;
	};
}
