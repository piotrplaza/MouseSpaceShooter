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
				const auto missileId = std::get<TCM::Missile>(Tools::AccessUserData(*missileFixture.GetBody()).bodyComponentVariant).id;
				const auto& targetBodyComponentVariant = Tools::AccessUserData(*targetFixture.GetBody()).bodyComponentVariant;
				const auto missilePlaneId = missilesToHandlers.at(missileId).planeId;

				if (!missilePlaneId)
					return true;

				if (std::holds_alternative<TCM::StaticPolyline>(targetBodyComponentVariant) || std::holds_alternative<TCM::DynamicPolyline>(targetBodyComponentVariant))
					return false;

				if (const TCM::Missile* targetMissile = std::get_if<TCM::Missile>(&targetBodyComponentVariant))
				{
					const auto targetMissilePlaneId = missilesToHandlers.at(targetMissile->id).planeId;
					return !targetMissilePlaneId || *missilePlaneId != *targetMissilePlaneId;
				}

				return *missilePlaneId != std::get<TCM::Plane>(targetBodyComponentVariant).id;
			});

		Globals::Components().beginCollisionHandlers().emplace(Globals::CollisionBits::missile, Globals::CollisionBits::all,
			[this](const auto& missileFixture, const auto& targetFixture) {
				auto& deferredActions = Globals::Components().deferredActions();
				const auto& missileBody = *missileFixture.GetBody();

				deferredActions.emplace([&](auto) {
					missilesToHandlers.erase(std::get<TCM::Missile>(Tools::AccessUserData(missileBody).bodyComponentVariant).id);
					return false;
					});

				const glm::vec2 explosionCenter = ToVec2<glm::vec2>(missileBody.GetWorldCenter());

				CreateExplosion(explosionParams.center(explosionCenter).explosionTexture(explosionTexture)
					.resolutionMode(resolutionModeF ? resolutionModeF(*targetFixture.GetBody()) : explosionParams.resolutionMode_));

				const auto& targetBodyComponentVariant = Tools::AccessUserData(*targetFixture.GetBody()).bodyComponentVariant;
				if (const TCM::Missile* targetMissile = std::get_if<TCM::Missile>(&targetBodyComponentVariant))
				{
					deferredActions.emplace([=](auto) {
						missilesToHandlers.erase(targetMissile->id);
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

	void MissilesHandler::setExplosionTexture(ComponentId explosionTexture)
	{
		this->explosionTexture = explosionTexture;
	}

	void MissilesHandler::setMissileTexture(ComponentId missileTexture)
	{
		this->missileTexture = missileTexture;
	}

	void MissilesHandler::setFlameAnimatedTexture(ComponentId flameAnimatedTexture)
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

	const MissileHandler* MissilesHandler::launchingMissile(unsigned playerHandlerId, bool tryToLaunch, std::optional<ComponentId> soundBufferId, float maxLifetime)
	{
		auto& playerHandler = playersHandler->accessPlayersHandlers()[playerHandlerId];

		if (tryToLaunch)
		{
			if (playerHandler.durationToLaunchMissile <= 0.0f)
			{
				playerHandler.durationToLaunchMissile = 0.1f;
				return &launchMissile(playerHandler.playerId, soundBufferId, maxLifetime);
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

	const MissileHandler& MissilesHandler::launchMissile(ComponentId playerId, std::optional<ComponentId> soundBufferId, float maxLifetime)
	{
		auto missileHandler = Tools::CreateMissile(Globals::Components().planes()[playerId].getOrigin2D(),
			Globals::Components().planes()[playerId].getAngle(), 5.0f, { 0.0f, 0.0f },
			Globals::Components().planes()[playerId].getVelocity(),
			missileTexture, flameAnimatedTexture, playerId, soundBufferId);

		auto& deferredActions = Globals::Components().deferredActions();
		deferredActions.emplace(
			[this, missileId = missileHandler.missileId, maxLifetime](float duration) {
				if (duration < maxLifetime)
					return true;
				missilesToHandlers.erase(missileId);
				return false;
			});

		return missilesToHandlers.emplace(missileHandler.missileId, std::move(missileHandler)).first->second;
	}
}
