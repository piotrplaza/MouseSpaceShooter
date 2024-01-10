#include "playersHandler.hpp"

#include "utility.hpp"

#include <components/gamepad.hpp>
#include <components/camera2D.hpp>
#include <components/plane.hpp>
#include <components/physics.hpp>
#include <components/screenInfo.hpp>
#include <components/sound.hpp>
#include <components/audioListener.hpp>
#include <components/mouse.hpp>

#include <globals/components.hpp>

#include <algorithm>

namespace
{
	ComponentId CreatePresettedPlane(unsigned id, ComponentId planeTexture, ComponentId flameAnimatedTexture, glm::vec3 initLoc)
	{
		switch (id)
		{
		case 0:
			return Tools::CreatePlane(Tools::CreateTrianglesBody({ { glm::vec2{2.0f, 0.0f}, glm::vec2{-1.0f, 1.0f}, glm::vec2{-1.0f, -1.0f} } }, Tools::GetDefaultParamsForPlaneBody()),
				planeTexture, flameAnimatedTexture, Tools::PlaneParams().position(initLoc).angle(initLoc.z));
		case 1:
			return Tools::CreatePlane(Tools::CreatePieBody(1.125f, 0.75f, glm::two_pi<float>() - 0.75f, 20, Tools::GetDefaultParamsForPlaneBody()),
				planeTexture, flameAnimatedTexture, Tools::PlaneParams().position(initLoc).angle(initLoc.z).numOfThrusts(1).thrustOffset({ -0.5f, 0.0f }).thrustAngle(0.0f));
		case 2:
			return Tools::CreatePlane(Tools::CreateTrianglesBody({ { glm::vec2{1.51f, 0.0f}, glm::vec2{-0.8f, 1.3f}, glm::vec2{-0.8f, -1.3f} } }, Tools::GetDefaultParamsForPlaneBody()),
				planeTexture, flameAnimatedTexture, Tools::PlaneParams().position(initLoc).angle(initLoc.z).thrustOffset({ 0.1f, 0.3f }));
		case 3:
			return Tools::CreatePlane(Tools::CreateBoxBody(glm::vec2{1.58f, 1.0f} * 0.6885f, Tools::GetDefaultParamsForPlaneBody()),
				planeTexture, flameAnimatedTexture, Tools::PlaneParams().position(initLoc).angle(initLoc.z).thrustOffset({ -0.7f, 0.1f }).thrustAngle(0.0f));
		}

		assert(!"wrong plane id");

		return 0;
	}

	std::optional<ComponentId> CreateAndPlayPlaneSound(std::optional<ComponentId> soundBuffer, ComponentId planeId)
	{
		if (!soundBuffer)
			return std::nullopt;

		const auto& plane = Globals::Components().planes()[planeId];

		auto& sound = Globals::Components().sounds().emplace(*soundBuffer);
		sound.setLoop(true);
		sound.setVolume(0.0f);
		sound.stepF = [&plane](auto& sound) {
			sound.setPosition(plane.getOrigin2D());
		};

		sound.play();

		return sound.getComponentId();
	}
}

namespace Tools
{
	PlayersHandler::~PlayersHandler()
	{
		auto& planes = Globals::Components().planes();
		auto& sounds = Globals::Components().sounds();

		for (const auto& playerHandler : playersHandlers)
		{
			planes[playerHandler.playerId].state = ComponentState::Outdated;

			if (playerHandler.grappleSound)
				sounds[*playerHandler.grappleSound].state = ComponentState::Outdated;

			if (playerHandler.thrustSound)
				sounds[*playerHandler.thrustSound].state = ComponentState::Outdated;
		}
	}

	void PlayersHandler::initPlayers(const std::array<ComponentId, 4>& planeTexturesForPlayers, const std::array<ComponentId, 4>& flameAnimatedTexturesForPlayers, bool gamepadForPlayer1,
		std::function<glm::vec3(unsigned playerId, unsigned numOfPlayers)> initLocF, bool centerToFront, std::optional<ComponentId> thrustSoundBuffer, std::optional<ComponentId> grappleSoundBuffer)
	{
		const auto& gamepads = Globals::Components().gamepads();
		auto& planes = Globals::Components().planes();
		auto& sounds = Globals::Components().sounds();

		std::erase_if(playersHandlers, [&](const auto& playerHandler) mutable {
			planes[playerHandler.playerId].setEnable(false);
			planes[playerHandler.playerId].state = ComponentState::Outdated;
			if (playerHandler.thrustSound)
				sounds[*playerHandler.thrustSound].state = ComponentState::Outdated;
			if (playerHandler.grappleSound)
				sounds[*playerHandler.grappleSound].state = ComponentState::Outdated;
				return true;
			});

		this->planeTextures = planeTexturesForPlayers;
		this->flameAnimatedTexturesForPlayers = flameAnimatedTexturesForPlayers;
		this->gamepadForPlayer1 = gamepadForPlayer1;
		this->thrustSoundBuffer = thrustSoundBuffer;
		this->grappleSoundBuffer = grappleSoundBuffer;

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
			const glm::vec3 initLoc = initLocF(i, numOfPlayers);
			ComponentId planeId = CreatePresettedPlane(i, planeTextures[i], flameAnimatedTexturesForPlayers[i], initLoc);
			playersHandlers.emplace_back(planeId, i == 0 && !gamepadForPlayer1 || activeGamepads.empty() ? std::nullopt : std::optional(activeGamepads[activeGamepadId++]),
				0.0f, CreateAndPlayPlaneSound(thrustSoundBuffer, planeId), 0.0f, CreateAndPlayPlaneSound(grappleSoundBuffer, planeId), 0.0f);

			if (centerToFront)
			{
				auto& plane = planes[planeId];
				plane.setOrigin(plane.getOrigin2D() - glm::vec2(std::cos(plane.getAngle()), std::sin(plane.getAngle())) * plane.getHorizontalOffsets()[1]);
			}
		}
	}

	void PlayersHandler::setCamera(CameraParams cameraParams) const
	{
		const auto& planes = Globals::Components().planes();
		const auto& screenInfo = Globals::Components().screenInfo();

		auto velocityCorrection = [velocityFactor = cameraParams.velocityFactor_](const auto& plane) {
			return plane.getVelocity() * velocityFactor;
		};

		Globals::Components().camera2D().targetProjectionHSizeF = [&, velocityCorrection,
				projectionHSizeMin = std::move(cameraParams.projectionHSizeMin_),
				scalingFactor = cameraParams.scalingFactor_,
				transitionFactor = cameraParams.transitionFactor_,
				additionalActors = cameraParams.additionalActors_]() {
			Globals::Components().camera2D().projectionTransitionFactor = Globals::Components().physics().frameDuration * transitionFactor;

			const float maxDistance = [&]() {
				const auto activePlayersHandlers = getActivePlayersHandlers();

				if (activePlayersHandlers.size() == 1 && additionalActors.empty())
					return glm::length(velocityCorrection(planes[activePlayersHandlers[0]->playerId])) + projectionHSizeMin();

				std::vector<glm::vec2> allActorsPos;
				allActorsPos.reserve(activePlayersHandlers.size() + additionalActors.size());

				for (const auto& playerHandler : activePlayersHandlers)
				{
					const auto& plane = planes[playerHandler->playerId];
					allActorsPos.emplace_back(plane.getOrigin2D() + velocityCorrection(plane));
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
						maxDistance = std::max(maxDistance, glm::distance(glm::vec2(pos1.x / screenInfo.getAspectRatio(), pos1.y),
							glm::vec2(pos2.x / screenInfo.getAspectRatio(), pos2.y)) * scalingFactor);
					}
				return maxDistance;
			}();

			return std::max(projectionHSizeMin(), maxDistance);
		};

		Globals::Components().camera2D().targetPositionF = [&, velocityCorrection, transitionFactor = cameraParams.transitionFactor_,
			additionalActors = cameraParams.additionalActors_]() {
			Globals::Components().camera2D().positionTransitionFactor = Globals::Components().physics().frameDuration * transitionFactor;

			glm::vec2 min(std::numeric_limits<float>::max());
			glm::vec2 max(std::numeric_limits<float>::lowest());
			glm::vec2 sumOfVelocityCorrections(0.0f);
			auto activePlayersHandlers = getActivePlayersHandlers();
			std::vector<glm::vec2> allActorsPos;
			allActorsPos.reserve(activePlayersHandlers.size() + additionalActors.size());

			for (const auto& playerHandler : activePlayersHandlers)
			{
				const auto& plane = planes[playerHandler->playerId];
				allActorsPos.emplace_back(plane.getOrigin2D());
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

	void PlayersHandler::gamepadsAutodetectionStep(std::function<glm::vec3(unsigned player)> initLocF)
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
					glm::vec3 initLoc = initLocF(playersHandlers.size());
					ComponentId planeId = CreatePresettedPlane(playersHandlers.size(), planeTextures[playersHandlers.size()], flameAnimatedTexturesForPlayers[playersHandlers.size()], initLoc);
					playersHandlers.emplace_back(planeId, activeGamepadId, 0.0f,
						CreateAndPlayPlaneSound(thrustSoundBuffer, planeId), 0.0f, CreateAndPlayPlaneSound(grappleSoundBuffer, planeId), 0.0f);
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
					Globals::Components().sounds()[*playerHandler.thrustSound].setVolume(0.0f);
				if (playerHandler.grappleSound)
					Globals::Components().sounds()[*playerHandler.grappleSound].setVolume(0.0f);
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
				playerControls.startPressed = mouse.pressed.mmb;
				fire = mouse.pressing.lmb;
			}

			if (gamepadId)
			{
				const auto& gamepad = gamepads[*gamepadId];

				playerControls.turningDelta += Tools::ApplyDeadzone(gamepad.lStick) * physics.frameDuration * gamepadSensitivity;
				playerControls.autoRotation |= (bool)gamepad.rTrigger;
				playerControls.throttling = std::max(gamepad.rTrigger, playerControls.throttling);
				playerControls.magneticHook |= gamepad.pressing.lShoulder || gamepad.pressing.a || gamepad.lTrigger >= 0.5f;
				playerControls.startPressed |= gamepad.pressed.start;
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

				Globals::Components().sounds()[*playerHandler.thrustSound].setVolume(playerHandler.thrustVolume);
			}

			if (playerHandler.grappleSound)
			{
				Globals::Components().sounds()[*playerHandler.grappleSound].setVolume((bool)plane.details.connectedGrappleId);
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
}
