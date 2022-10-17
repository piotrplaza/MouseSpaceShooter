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

	void PlayersHandler::initMultiplayerCamera(std::function<float()> projectionHSizeMin, float scalingFactor, float velocityFactor, float transitionFactor) const
	{
		const auto& planes = Globals::Components().planes();
		const auto& screenInfo = Globals::Components().screenInfo();

		auto velocityCorrection = [velocityFactor](const auto& plane) {
			return plane.getVelocity() * velocityFactor;
		};

		Globals::Components().camera().targetProjectionHSizeF = [&, velocityCorrection, projectionHSizeMin, scalingFactor, transitionFactor]() {
			Globals::Components().camera().projectionTransitionFactor = Globals::Components().physics().frameDuration * transitionFactor;

			const float maxDistance = [&, velocityCorrection, scalingFactor]() {
				if (playersHandlers.size() == 1)
					return glm::length(velocityCorrection(planes[playersHandlers[0].playerId])) + projectionHSizeMin();

				float maxDistance = 0.0f;
				for (unsigned i = 0; i < playersHandlers.size() - 1; ++i)
					for (unsigned j = i + 1; j < playersHandlers.size(); ++j)
					{
						const auto& plane1 = planes[playersHandlers[i].playerId];
						const auto& plane2 = planes[playersHandlers[j].playerId];
						const auto plane1Center = plane1.getCenter() + velocityCorrection(plane1);
						const auto plane2Center = plane2.getCenter() + velocityCorrection(plane2);
						/*maxDistance = std::max(maxDistance, glm::max(glm::abs(plane1Center.x - plane2Center.x) * screenInfo.windowSize.y / screenInfo.windowSize.x * scalingFactor,
							glm::abs(plane1Center.y - plane2Center.y) * scalingFactor));*/
						maxDistance = std::max(maxDistance, glm::distance(glm::vec2(plane1Center.x * screenInfo.windowSize.y / screenInfo.windowSize.x, plane1Center.y),
							glm::vec2(plane2Center.x * screenInfo.windowSize.y / screenInfo.windowSize.x, plane2Center.y)) * scalingFactor);
					}
				return maxDistance;
			}();

			return std::max(projectionHSizeMin(), maxDistance);
		};

		Globals::Components().camera().targetPositionF = [&, velocityCorrection, transitionFactor]() {
			Globals::Components().camera().positionTransitionFactor = Globals::Components().physics().frameDuration * transitionFactor;

			glm::vec2 min(std::numeric_limits<float>::max());
			glm::vec2 max(std::numeric_limits<float>::lowest());
			glm::vec2 sumOfVelocityCorrections(0.0f);

			for (const auto& playerHandler : playersHandlers)
			{
				const auto& plane = planes[playerHandler.playerId];
				min = { std::min(min.x, plane.getCenter().x), std::min(min.y, plane.getCenter().y) };
				max = { std::max(max.x, plane.getCenter().x), std::max(max.y, plane.getCenter().y) };
				sumOfVelocityCorrections += velocityCorrection(plane);
			}

			return (min + max) / 2.0f + sumOfVelocityCorrections / (float)playersHandlers.size();
		};
	}

	void PlayersHandler::autodetectionStep(std::function<glm::vec2(unsigned player)> initPosF)
	{
		const auto& gamepads = Globals::Components().gamepads();
		auto& planes = Globals::Components().planes();
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
