#include "playersHandler.hpp"

#include "utility.hpp"

#include <components/gamepad.hpp>
#include <components/camera.hpp>
#include <components/plane.hpp>
#include <components/physics.hpp>
#include <components/screenInfo.hpp>
#include <components/sound.hpp>
#include <components/mouse.hpp>

#include <globals/components.hpp>

namespace Tools
{
	void PlayersHandler::initPlayers(ComponentId rocketPlaneTexture, const std::array<unsigned, 4>& flameAnimatedTextureForPlayers, bool gamepadForPlayer1,
		std::function<glm::vec2(unsigned player, unsigned numOfPlayers)> initPosF, std::optional<ComponentId> thrustSoundBuffer, std::optional<ComponentId> grappleSoundBuffer)
	{
		this->rocketPlaneTexture = rocketPlaneTexture;
		this->flameAnimatedTextureForPlayers = flameAnimatedTextureForPlayers;
		this->gamepadForPlayer1 = gamepadForPlayer1;
		this->thrustSoundBuffer = thrustSoundBuffer;
		this->grappleSoundBuffer = grappleSoundBuffer;

		const auto& gamepads = Globals::Components().gamepads();
		std::vector<unsigned> activeGamepads;

		for (unsigned i = 0; i < gamepads.size(); ++i)
		{
			const auto& gamepad = gamepads[i];
			if (gamepad.enabled)
				activeGamepads.push_back(i);
		}

		const unsigned numOfPlayers = std::clamp(activeGamepads.size() + !gamepadForPlayer1, 1u, 4u);

		playersHandlers.reserve(numOfPlayers);
		unsigned activeGamepadId = 0;
		for (unsigned i = 0; i < numOfPlayers; ++i)
		{
			ComponentId planeId = Tools::CreatePlane(rocketPlaneTexture, flameAnimatedTextureForPlayers[i], initPosF(i, numOfPlayers));
			playersHandlers.emplace_back(planeId, i == 0 && !gamepadForPlayer1 || activeGamepads.empty() ? std::nullopt : std::optional(activeGamepads[activeGamepadId++]),
				0.0f, createSound(thrustSoundBuffer, planeId), 0.0f, createSound(grappleSoundBuffer, planeId), 0.0f);
		}
	}

	void PlayersHandler::setCamera(CameraParams cameraParams) const
	{
		const auto& planes = Globals::Components().planes();
		const auto& screenInfo = Globals::Components().screenInfo();

		auto velocityCorrection = [velocityFactor = cameraParams.velocityFactor_](const auto& plane) {
			return plane.getVelocity() * velocityFactor;
		};

		Globals::Components().camera().targetProjectionHSizeF = [&, velocityCorrection,
				projectionHSizeMin = std::move(cameraParams.projectionHSizeMin_),
				scalingFactor = cameraParams.scalingFactor_,
				transitionFactor = cameraParams.transitionFactor_,
				additionalActors = cameraParams.additionalActors_]() {
			Globals::Components().camera().projectionTransitionFactor = Globals::Components().physics().frameDuration * transitionFactor;

			const float maxDistance = [&]() {
				const auto activePlayersHandlers = getActivePlayersHandlers();

				if (activePlayersHandlers.size() == 1 && additionalActors.empty())
					return glm::length(velocityCorrection(planes[activePlayersHandlers[0]->playerId])) + projectionHSizeMin();

				std::vector<glm::vec2> allActorsPos;
				allActorsPos.reserve(activePlayersHandlers.size() + additionalActors.size());

				for (const auto& playerHandler : activePlayersHandlers)
				{
					const auto& plane = planes[playerHandler->playerId];
					allActorsPos.emplace_back(plane.getCenter() + velocityCorrection(plane));
				}
				for (const auto& additionalActor : additionalActors)
					allActorsPos.emplace_back(additionalActor());

				float maxDistance = 0.0f;
				for (unsigned i = 0; i < allActorsPos.size() - 1; ++i)
					for (unsigned j = i + 1; j < allActorsPos.size(); ++j)
					{
						const auto pos1 = allActorsPos[i];
						const auto pos2 = allActorsPos[j];
						/*maxDistance = std::max(maxDistance, glm::max(glm::abs(pos1.x - pos2.x) * screenInfo.windowSize.y / screenInfo.windowSize.x * scalingFactor,
							glm::abs(pos1.y - pos2.y) * scalingFactor));*/
						maxDistance = std::max(maxDistance, glm::distance(glm::vec2(pos1.x * screenInfo.windowSize.y / screenInfo.windowSize.x, pos1.y),
							glm::vec2(pos2.x * screenInfo.windowSize.y / screenInfo.windowSize.x, pos2.y)) * scalingFactor);
					}
				return maxDistance;
			}();

			return std::max(projectionHSizeMin(), maxDistance);
		};

		Globals::Components().camera().targetPositionF = [&, velocityCorrection, transitionFactor = cameraParams.transitionFactor_,
			additionalActors = cameraParams.additionalActors_]() {
			Globals::Components().camera().positionTransitionFactor = Globals::Components().physics().frameDuration * transitionFactor;

			glm::vec2 min(std::numeric_limits<float>::max());
			glm::vec2 max(std::numeric_limits<float>::lowest());
			glm::vec2 sumOfVelocityCorrections(0.0f);
			auto activePlayersHandlers = getActivePlayersHandlers();
			std::vector<glm::vec2> allActorsPos;
			allActorsPos.reserve(activePlayersHandlers.size() + additionalActors.size());

			for (const auto& playerHandler : activePlayersHandlers)
			{
				const auto& plane = planes[playerHandler->playerId];
				allActorsPos.emplace_back(plane.getCenter());
				sumOfVelocityCorrections += velocityCorrection(plane);
			}
			for (const auto& additionalActor : additionalActors)
				allActorsPos.emplace_back(additionalActor());

			for (const auto& actorPos : allActorsPos)
			{
				min = { std::min(min.x, actorPos.x), std::min(min.y, actorPos.y) };
				max = { std::max(max.x, actorPos.x), std::max(max.y, actorPos.y) };
			}

			return (min + max) / 2.0f + sumOfVelocityCorrections / (float)activePlayersHandlers.size();
		};
	}

	void PlayersHandler::autodetectionStep(std::function<glm::vec2(unsigned player)> initPosF)
	{
		const auto& gamepads = Globals::Components().gamepads();
		auto& planes = Globals::Components().planes();
		auto& sounds = Globals::Components().sounds();
		std::vector<unsigned> activeGamepads;

		for (unsigned i = 0; i < gamepads.size(); ++i)
		{
			const auto& gamepad = gamepads[i];
			if (gamepad.enabled)
				activeGamepads.push_back(i);
		}

		const unsigned numOfPlayers = std::clamp(activeGamepads.size() + !gamepadForPlayer1, 1u, 4u);

		std::erase_if(playersHandlers, [&, i = 0](const auto& playerHandler) mutable {
			const bool erase = i++ != 0 && playerHandler.gamepadId && !gamepads[*playerHandler.gamepadId].enabled;
			if (erase)
			{
				planes[playerHandler.playerId].state = ComponentState::Outdated;
				if (playerHandler.thrustSound)
					sounds[*playerHandler.thrustSound].state = ComponentState::Outdated;
				if (playerHandler.grappleSound)
					sounds[*playerHandler.grappleSound].state = ComponentState::Outdated;
				return true;
			}
			return false;
			});

		for (auto activeGamepadId : activeGamepads)
		{
			auto it = std::find_if(playersHandlers.begin(), playersHandlers.end(), [&](const auto& playerHandler) {
				return playerHandler.gamepadId && playerHandler.gamepadId == activeGamepadId;
				});
			if (it == playersHandlers.end())
				if (gamepadForPlayer1 && !playersHandlers[0].gamepadId)
					playersHandlers[0].gamepadId = activeGamepadId;
				else if (playersHandlers.size() < 4)
				{
					ComponentId planeId = Tools::CreatePlane(rocketPlaneTexture, flameAnimatedTextureForPlayers[playersHandlers.size()],
						initPosF(playersHandlers.size()));
					playersHandlers.emplace_back(planeId, activeGamepadId, 0.0f,
						createSound(thrustSoundBuffer, planeId), 0.0f, createSound(grappleSoundBuffer, planeId), 0.0f);
				}
		}
	}

	void PlayersHandler::controlStep(std::function<void(unsigned playerHandlerId, bool fire)> fireF)
	{
		const float mouseSensitivity = 0.01f;
		const float gamepadSensitivity = 50.0f;

		const auto& physics = Globals::Components().physics();
		const auto& mouse = Globals::Components().mouse();
		const auto& gamepads = Globals::Components().gamepads();

		for (unsigned i = 0; i < playersHandlers.size(); ++i)
		{
			auto& playerHandler = playersHandlers[i];
			auto& plane = Globals::Components().planes()[playerHandler.playerId];

			if (!plane.isEnabled())
			{
				if (playerHandler.thrustSound)
					Globals::Components().sounds()[*playerHandler.thrustSound].volume(0.0f);
				if (playerHandler.grappleSound)
					Globals::Components().sounds()[*playerHandler.grappleSound].volume(0.0f);
				continue;
			}

			const auto gamepadId = playerHandler.gamepadId;
			auto& playerControls = plane.controls;
			bool fire = false;

			playerControls = Components::Plane::Controls();

			if (i == 0)
			{
				playerControls.turningDelta = mouse.getWorldSpaceDelta() * mouseSensitivity;
				playerControls.autoRotation = (bool)mouse.pressing.rmb;
				playerControls.throttling = (float)mouse.pressing.rmb;
				playerControls.magneticHook = mouse.pressing.xmb1;
				fire = mouse.pressing.lmb;
			}

			if (gamepadId)
			{
				const auto& gamepad = gamepads[*gamepadId];

				playerControls.turningDelta += Tools::ApplyDeadzone(gamepad.lStick) * physics.frameDuration * gamepadSensitivity;
				playerControls.autoRotation |= (bool)gamepad.rTrigger;
				playerControls.throttling = std::max(gamepad.rTrigger, playerControls.throttling);
				playerControls.magneticHook |= gamepad.pressing.lShoulder || gamepad.pressing.a || gamepad.lTrigger >= 0.5f;
				fire |= gamepad.pressing.x;
			}

			if (fireF)
				fireF(i, fire);

			if (playerHandler.thrustSound)
			{
				const float volumeStep = physics.frameDuration * 2.0f;

				if (playerControls.throttling > playerHandler.thrustVolume)
				{
					playerHandler.thrustVolume += volumeStep;
					playerHandler.thrustVolume = std::min(playerHandler.thrustVolume, playerControls.throttling);
				}
				else
				{
					playerHandler.thrustVolume -= volumeStep;
					playerHandler.thrustVolume = std::max(playerHandler.thrustVolume, 0.0f);
				}

				Globals::Components().sounds()[*playerHandler.thrustSound].volume(playerHandler.thrustVolume);
			}

			if (playerHandler.grappleSound)
			{
				Globals::Components().sounds()[*playerHandler.grappleSound].volume((bool)plane.details.connectedGrappleId);
			}
		}
	}

	const std::vector<Tools::PlayerHandler>& PlayersHandler::getPlayersHandlers() const
	{
		return playersHandlers;
	}

	std::vector<Tools::PlayerHandler>& PlayersHandler::accessPlayersHandlers()
	{
		return playersHandlers;
	}

	const std::vector<const Tools::PlayerHandler*> PlayersHandler::getActivePlayersHandlers() const
	{
		std::vector<const Tools::PlayerHandler*> activePlayersHandlers;

		for (const auto& playerHandler : playersHandlers)
			if (Globals::Components().planes()[playerHandler.playerId].isEnabled())
				activePlayersHandlers.push_back(&playerHandler);

		return activePlayersHandlers;
	}

	std::vector<Tools::PlayerHandler*> PlayersHandler::accessActivePlayersHandlers()
	{
		std::vector<Tools::PlayerHandler*> activePlayersHandlers;

		for (auto& playerHandler : playersHandlers)
			if (Globals::Components().planes()[playerHandler.playerId].isEnabled())
				activePlayersHandlers.push_back(&playerHandler);

		return activePlayersHandlers;
	}

	std::optional<ComponentId> PlayersHandler::createSound(std::optional<ComponentId> soundBuffer, ComponentId planeId) const
	{
		if (!soundBuffer)
			return std::nullopt;

		const auto& plane = Globals::Components().planes()[planeId];

		auto& sound = Globals::Components().sounds().emplace(*soundBuffer);
		sound.loop(true);
		sound.volume(0.0f);
		sound.stepF = [&plane](auto& sound) {
			sound.position(Tools::GetRelativePos(plane.getCenter()));
		};
		sound.play();

		return sound.getComponentId();
	}
}
