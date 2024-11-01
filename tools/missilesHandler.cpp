#include "missilesHandler.hpp"

#include "playersHandler.hpp"

#include <components/physics.hpp>
#include <components/plane.hpp>
#include <components/collisionHandler.hpp>
#include <components/collisionFilter.hpp>
#include <components/deferredAction.hpp>

#include <globals/components.hpp>
#include <globals/collisionBits.hpp>

namespace Tools
{
	void MissilesHandler::initCollisions()
	{
		Globals::Components().collisionFilters().emplace(Globals::CollisionBits::missile, Globals::CollisionBits::missile |
			Globals::CollisionBits::plane | Globals::CollisionBits::polyline,
			[this](const auto& missileFixture, const auto& targetFixture) {
				const auto missile = std::get<CM::Missile>(Tools::AccessUserData(*missileFixture.GetBody()).bodyComponentVariant);
				const auto& targetBodyComponentVariant = Tools::AccessUserData(*targetFixture.GetBody()).bodyComponentVariant;
				const auto missilePlane = missilesToHandlers.at(missile).plane;

				if (!missilePlane)
					return true;

				if (std::holds_alternative<CM::Polyline>(targetBodyComponentVariant))
					return false;

				if (const CM::Missile* targetMissile = std::get_if<CM::Missile>(&targetBodyComponentVariant))
				{
					const auto targetMissilePlane = missilesToHandlers.at(targetMissile->componentId).plane;
					return !targetMissilePlane || *missilePlane != *targetMissilePlane;
				}

				return *missilePlane != std::get<CM::Plane>(targetBodyComponentVariant);
			});

		Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::missile, Globals::CollisionBits::all,
			[this](const auto& missileFixture, const auto& targetFixture) {
				auto& deferredActions = Globals::Components().deferredActions();
				const auto& missileBody = *missileFixture.GetBody();

				deferredActions.emplace([&](auto) {
					missilesToHandlers.erase(std::get<CM::Missile>(Tools::AccessUserData(missileBody).bodyComponentVariant).componentId);
					return false;
					});

				const glm::vec2 explosionCenter = ToVec2<glm::vec2>(missileBody.GetWorldCenter());

				CreateExplosion(explosionParams.center(explosionCenter).explosionTexture(explosionTexture)
					.resolutionMode(resolutionModeF ? resolutionModeF(*targetFixture.GetBody()) : explosionParams.resolutionMode_));

				const auto& targetBodyComponentVariant = Tools::AccessUserData(*targetFixture.GetBody()).bodyComponentVariant;
				if (const CM::Missile* targetMissile = std::get_if<CM::Missile>(&targetBodyComponentVariant))
				{
					deferredActions.emplace([=](auto) {
						missilesToHandlers.erase(targetMissile->componentId);
						return false;
						});

					CreateExplosion(explosionParams.center(ToVec2<glm::vec2>(targetFixture.GetBody()->GetWorldCenter())).explosionTexture(explosionTexture));
				}

				if (explosionF)
					explosionF(explosionCenter);
			});
	}

	void MissilesHandler::setPlayersHandler(Tools::PlayersHandler& playersHandler)
	{
		this->playersHandler = &playersHandler;
	}

	void MissilesHandler::setExplosionTexture(CM::Texture explosionTexture)
	{
		this->explosionTexture = explosionTexture;
	}

	void MissilesHandler::setMissileTexture(CM::Texture missileTexture)
	{
		this->missileTexture = missileTexture;
	}

	void MissilesHandler::setFlameAnimatedTexture(CM::AnimatedTexture flameAnimatedTexture)
	{
		this->flameAnimatedTexture = flameAnimatedTexture;
	}

	void MissilesHandler::setExplosionParams(Tools::ExplosionParams explosionParams)
	{
		this->explosionParams = explosionParams;
	}

	void MissilesHandler::setResolutionModeF(std::function<ResolutionMode(const b2Body&)> resolutionModeF)
	{
		this->resolutionModeF = resolutionModeF;
	}

	void MissilesHandler::setExplosionF(std::function<void(glm::vec2)> explosionF)
	{
		this->explosionF = explosionF;
	}

	const MissileHandler* MissilesHandler::launchingMissile(unsigned playerHandlerId, bool tryToLaunch, std::optional<CM::SoundBuffer> soundBuffer, float maxLifetime)
	{
		auto& playerHandler = playersHandler->accessPlayersHandlers()[playerHandlerId];

		if (tryToLaunch)
		{
			if (playerHandler.durationToLaunchMissile <= 0.0f)
			{
				playerHandler.durationToLaunchMissile = 0.1f;
				return &launchMissile(playerHandler.playerId, soundBuffer, maxLifetime);
			}
			else playerHandler.durationToLaunchMissile -= Globals::Components().physics().frameDuration;
		}
		else playerHandler.durationToLaunchMissile = 0.0f;

		return nullptr;
	}

	void MissilesHandler::removeActiveMissiles()
	{
		missilesToHandlers.clear();
	}

	const MissileHandler& MissilesHandler::launchMissile(ComponentId playerId, std::optional<CM::SoundBuffer> soundBuffer, float maxLifetime)
	{
		auto missileHandler = Tools::CreateMissile(Globals::Components().planes()[playerId].getOrigin2D(),
			Globals::Components().planes()[playerId].getAngle(), 5.0f, { 0.0f, 0.0f },
			Globals::Components().planes()[playerId].getVelocity(),
			missileTexture, flameAnimatedTexture, playerId, soundBuffer);

		auto& deferredActions = Globals::Components().deferredActions();
		deferredActions.emplace(
			[this, missile = missileHandler.missile, maxLifetime](float duration) {
				if (duration < maxLifetime)
					return true;
				missilesToHandlers.erase(missile);
				return false;
			});

		return missilesToHandlers.emplace(missileHandler.missile, std::move(missileHandler)).first->second;
	}
}
