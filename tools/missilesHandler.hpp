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
		void initCollisions();
		void setPlayersHandler(Tools::PlayersHandler& playersHandler);
		void setExplosionTexture(CM::Texture explosionTexture);
		void setMissileTexture(CM::Texture missileTexture);
		void setFlameAnimatedTexture(CM::AnimatedTexture flameAnimatedTexture);

		void setExplosionParams(Tools::ExplosionParams explosionParams);

		void setResolutionModeF(std::function<ResolutionMode(const b2Body&)> resolutionModeF);
		void setExplosionF(std::function<void(glm::vec2)> explosionF);

		const MissileHandler* launchingMissile(unsigned playerHandlerId, bool tryToLaunch, std::optional<CM::SoundBuffer> soundBuffer = std::nullopt,
			float maxLifetime = 5.0f);

		void removeActiveMissiles();

	private:
		const MissileHandler& launchMissile(ComponentId playerId, std::optional<CM::SoundBuffer> soundBuffer = std::nullopt, float maxLifetime = 5.0f);

		std::unordered_map<CM::Missile, Tools::MissileHandler> missilesToHandlers;

		Tools::PlayersHandler* playersHandler = nullptr;

		CM::Texture explosionTexture;
		CM::Texture missileTexture;
		CM::AnimatedTexture flameAnimatedTexture;

		Tools::ExplosionParams explosionParams;

		std::function<ResolutionMode(const b2Body&)> resolutionModeF;
		std::function<void(glm::vec2)> explosionF;
	};
}
