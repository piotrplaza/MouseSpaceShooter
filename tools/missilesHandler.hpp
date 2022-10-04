#pragma once

#include <commonTypes/componentId.hpp>

#include "gameHelpers.hpp"

#include <glm/vec2.hpp>

#include <optional>

namespace Tools
{
	class PlayersHandler;

	class MissilesHandler
	{
	public:
		MissilesHandler();

		void setPlayersHandler(Tools::PlayersHandler& playersHandler);
		void setExplosionTexture(ComponentId explosionTexture);
		void setMissileTexture(ComponentId missileTexture);
		void setFlameAnimatedTexture(ComponentId flameAnimatedTexture);

		void setExplosionParams(Tools::ExplosionParams explosionParams);

		void setResolutionModeF(std::function<ResolutionMode(const b2Body&)> resolutionModeF);
		void setExplosionF(std::function<void(glm::vec2)> explosionF);

		const MissileHandler* launchingMissile(unsigned playerHandlerId, bool tryToLaunch, std::optional<ComponentId> soundBufferId = std::nullopt,
			float maxLifetime = 5.0f);

	private:
		const MissileHandler& launchMissile(ComponentId playerId, std::optional<ComponentId> soundBufferId = std::nullopt, float maxLifetime = 5.0f);

		std::unordered_map<ComponentId, Tools::MissileHandler> missilesToHandlers;

		Tools::PlayersHandler* playersHandler = nullptr;

		ComponentId explosionTexture = 0;
		ComponentId missileTexture = 0;
		ComponentId flameAnimatedTexture = 0;

		Tools::ExplosionParams explosionParams;

		std::function<ResolutionMode(const b2Body&)> resolutionModeF;
		std::function<void(glm::vec2)> explosionF;
	};
}
