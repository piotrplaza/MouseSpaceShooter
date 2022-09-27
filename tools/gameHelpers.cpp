#include "gameHelpers.hpp"

#include <systems/deferredActions.hpp>

#include <components/plane.hpp>
#include <components/physics.hpp>
#include <components/decoration.hpp>
#include <components/missile.hpp>
#include <components/shockwave.hpp>
#include <components/renderingSetup.hpp>
#include <components/mvp.hpp>
#include <components/camera.hpp>
#include <components/screenInfo.hpp>
#include <components/animatedTexture.hpp>
#include <components/deferredAction.hpp>
#include <components/graphicsSettings.hpp>
#include <components/collisionFilter.hpp>
#include <components/collisionHandler.hpp>
#include <components/mouse.hpp>
#include <components/gamepad.hpp>

#include <commonTypes/typeComponentMappers.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>
#include <globals/collisionBits.hpp>

#include <ogl/uniforms.hpp>
#include <ogl/shaders/textured.hpp>
#include <ogl/shaders/julia.hpp>

#include <tools/b2Helpers.hpp>
#include <tools/utility.hpp>

#include <algorithm>
#include <cassert>

namespace Tools
{
	void PlayersHandler::initPlayers(unsigned rocketPlaneTexture, const std::array<unsigned, 4>& flameAnimatedTextureForPlayers, bool gamepadForPlayer1,
		std::function<glm::vec2(unsigned player, unsigned numOfPlayers)> initPosF)
	{
		this->rocketPlaneTexture = rocketPlaneTexture;
		this->flameAnimatedTextureForPlayers = flameAnimatedTextureForPlayers;
		this->gamepadForPlayer1 = gamepadForPlayer1;

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
			playersHandlers.emplace_back(Tools::CreatePlane(rocketPlaneTexture, flameAnimatedTextureForPlayers[i], initPosF(i, numOfPlayers)),
				i == 0 && !gamepadForPlayer1 || activeGamepads.empty() ? std::nullopt : std::optional(activeGamepads[activeGamepadId++]), 0.0f);
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
					playersHandlers.emplace_back(Tools::CreatePlane(rocketPlaneTexture, flameAnimatedTextureForPlayers[playersHandlers.size()],
						initPosF(playersHandlers.size())), activeGamepadId, 0.0f);
		}
	}

	void PlayersHandler::controlStep(std::function<void(unsigned playerHandlerId, bool fire)> fireF) const
	{
		const float mouseSensitivity = 0.01f;
		const float gamepadSensitivity = 50.0f;

		const auto& physics = Globals::Components().physics();
		const auto& mouse = Globals::Components().mouse();
		const auto& gamepads = Globals::Components().gamepads();

		for (unsigned i = 0; i < playersHandlers.size(); ++i)
		{
			const auto gamepadId = playersHandlers[i].gamepadId;
			auto& playerControls = Globals::Components().planes()[playersHandlers[i].playerId].controls;
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

	MissileHandler::MissileHandler() = default;

	MissileHandler::MissileHandler(ComponentId missileId, ComponentId backThrustId, glm::vec2 referenceVelocity, std::optional<ComponentId> planeId):
		missileId(missileId),
		backThrustId(backThrustId),
		referenceVelocity(referenceVelocity),
		planeId(planeId)
	{
	}

	MissileHandler::~MissileHandler()
	{
		if (!valid) return;

		auto missileIt = Globals::Components().missiles().find(missileId);
		assert(missileIt != Globals::Components().missiles().end());
		missileIt->state = ComponentState::Outdated;
		auto thrustIt = Globals::Components().dynamicDecorations().find(backThrustId);
		assert(thrustIt != Globals::Components().dynamicDecorations().end());
		thrustIt->state = ComponentState::Outdated;
	}

	MissileHandler::MissileHandler(MissileHandler&& other) noexcept:
		missileId(other.missileId),
		backThrustId(other.backThrustId),
		referenceVelocity(other.referenceVelocity),
		planeId(other.planeId)
	{
		other.valid = false;
	}

	MissileHandler& MissileHandler::operator=(MissileHandler&& other) noexcept
	{
		missileId = other.missileId;
		backThrustId = other.backThrustId;
		referenceVelocity = other.referenceVelocity;
		planeId = other.planeId;
		other.valid = false;
		return *this;
	}

	MissilesHandler::MissilesHandler()
	{
		Globals::Components().collisionFilters().emplace(Globals::CollisionBits::missile, Globals::CollisionBits::missile |
			Globals::CollisionBits::plane | Globals::CollisionBits::polyline,
			[this](const auto& missileFixture, const auto& targetFixture) {
				const auto missileId = std::get<TCM::Missile>(Tools::AccessUserData(*missileFixture.GetBody()).bodyComponentVariant).id;
				const auto& targetBodyComponentVariant = Tools::AccessUserData(*targetFixture.GetBody()).bodyComponentVariant;
				const auto missilePlaneId = missilesToHandlers.at(missileId).planeId;

				if (!missilePlaneId)
					return true;

				if (std::holds_alternative<TCM::Polyline>(targetBodyComponentVariant))
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

				CreateExplosion(explosionParams.center(ToVec2<glm::vec2>(missileBody.GetWorldCenter())).explosionTexture(explosionTexture)
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
					explosionF();
			});
	}

	void MissilesHandler::setPlayersHandler(Tools::PlayersHandler& playersHandler)
	{
		this->playersHandler = &playersHandler;
	}

	void MissilesHandler::setExplosionTexture(unsigned explosionTexture)
	{
		this->explosionTexture = explosionTexture;
	}

	void MissilesHandler::setMissileTexture(unsigned missileTexture)
	{
		this->missileTexture = missileTexture;
	}

	void MissilesHandler::setFlameAnimatedTexture(unsigned flameAnimatedTexture)
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

	void MissilesHandler::setExplosionF(std::function<void()> explosionF)
	{
		this->explosionF = explosionF;
	}

	void MissilesHandler::launchingMissile(unsigned playerHandlerId, bool tryToLaunch)
	{
		auto& playerHandler = playersHandler->accessPlayersHandlers()[playerHandlerId];
		if (tryToLaunch)
		{
			if (playerHandler.durationToLaunchMissile <= 0.0f)
			{
				launchMissile(playerHandler.playerId);
				playerHandler.durationToLaunchMissile = 0.1f;
			}
			else playerHandler.durationToLaunchMissile -= Globals::Components().physics().frameDuration;
		}
		else playerHandler.durationToLaunchMissile = 0.0f;
	}

	void MissilesHandler::launchMissile(unsigned playerId)
	{
		auto missileHandler = Tools::CreateMissile(Globals::Components().planes()[playerId].getCenter(),
			Globals::Components().planes()[playerId].getAngle(), 5.0f, { 0.0f, 0.0f },
			Globals::Components().planes()[playerId].getVelocity(),
			missileTexture, flameAnimatedTexture, playerId);
		missilesToHandlers.emplace(missileHandler.missileId, std::move(missileHandler));
	}

	ComponentId CreatePlane(unsigned planeTexture, unsigned flameAnimatedTexture, glm::vec2 position, float angle)
	{
		auto& plane = Globals::Components().planes().emplace(Tools::CreatePlaneBody(2.0f, 0.2f, 0.5f), TCM::Texture(planeTexture));

		auto& body = *plane.body;
		body.SetBullet(true);

		plane.setPosition(position);
		plane.setRotation(angle);
		plane.preserveTextureRatio = true;
		plane.posInSubsequence = 2;

		for (int i = 0; i < 2; ++i)
		{
			auto& planeDecoration = plane.subsequence.emplace_back(Tools::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 0.5f, 0.5f }), std::vector<glm::vec2>{},
				TCM::AnimatedTexture(flameAnimatedTexture));

			Globals::Components().renderingSetups().emplace([&, i,
				modelUniform = Uniforms::UniformMat4f(),
				thrust = 1.0f,
				flameAnimatedTexture
			](Shaders::ProgramId program) mutable {
					if (!modelUniform.isValid()) modelUniform = Uniforms::UniformMat4f(program, "model");
					modelUniform(
						glm::translate(
							glm::scale(
								glm::rotate(
									glm::translate(Tools::GetModelMatrix(*plane.body), { -0.5f, i == 0 ? -0.4f : 0.4f, 0.0f }),
									-glm::half_pi<float>() + (i == 0 ? 0.1f : -0.1f), { 0.0f, 0.0f, 1.0f }),
								{ 0.5f + thrust * 0.02f, thrust, 1.0f }),
							{ 0.0f, -0.5f, 0.0f }));

					const float targetThrust = 1.0f + plane.details.throttleForce * 0.3f;
					const float changeStep = Globals::Components().physics().frameDuration * 10.0f;

					if (thrust < targetThrust)
						thrust = std::min(thrust + changeStep, targetThrust);
					else
						thrust = std::max(thrust - changeStep, targetThrust);

					Globals::Components().animatedTextures()[flameAnimatedTexture].setSpeedScaling(1.0f + (thrust - 1) * 0.2f);

					glBlendFunc(GL_SRC_ALPHA, GL_ONE);

					return []() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); };
				});

			planeDecoration.renderingSetup = Globals::Components().renderingSetups().size() - 1;
		}

		return plane.getComponentId();
	}

	MissileHandler CreateMissile(glm::vec2 startPosition, float startAngle, float force, glm::vec2 referenceVelocity,
		glm::vec2 initialVelocity, unsigned missileTexture, unsigned flameAnimatedTexture, std::optional<ComponentId> planeId)
	{
		auto& missile = Globals::Components().missiles().emplace(Tools::CreateBoxBody({ 0.5f, 0.2f },
			Tools::BodyParams().position(startPosition).angle(startAngle).bodyType(b2_dynamicBody).density(0.2f)), force);

		auto& body = *missile.body;
		SetCollisionFilteringBits(body, Globals::CollisionBits::missile, Globals::CollisionBits::all - Globals::CollisionBits::missile - Globals::CollisionBits::plane);
		body.SetLinearVelocity(ToVec2<b2Vec2>(referenceVelocity + initialVelocity));
		body.SetBullet(true);

		missile.texture = TCM::Texture(missileTexture);
		missile.preserveTextureRatio = true;

		Globals::Components().renderingSetups().emplace([
			modelUniform = Uniforms::UniformMat4f(), &body](Shaders::ProgramId program) mutable
			{
				if (!modelUniform.isValid()) modelUniform = Uniforms::UniformMat4f(program, "model");
				modelUniform(Tools::GetModelMatrix(body));
				return nullptr;
			});

		missile.renderingSetup = Globals::Components().renderingSetups().size() - 1;
		missile.renderLayer = RenderLayer::FarMidground;

		auto& decoration = Globals::Components().dynamicDecorations().emplace(Tools::CreateVerticesOfRectangle({ 0.0f, -0.5f }, { 0.5f, 0.5f }),
			TCM::AnimatedTexture(flameAnimatedTexture), Tools::CreateTexCoordOfRectangle());

		Globals::Components().renderingSetups().emplace([&, modelUniform = Uniforms::UniformMat4f(),
			thrustScale = 0.1f
		](Shaders::ProgramId program) mutable {
				if (!modelUniform.isValid()) modelUniform = Uniforms::UniformMat4f(program, "model");
				modelUniform(glm::scale(glm::rotate(glm::translate(Tools::GetModelMatrix(*missile.body),
					{ -0.5f, 0.0f, 0.0f }),
					-glm::half_pi<float>(), { 0.0f, 0.0f, 1.0f }),
					{ std::min(thrustScale * 0.2f, 0.3f), thrustScale, 1.0f }));

				const float targetFrameDurationFactor = Globals::Components().physics().frameDuration * 6.0f;
				thrustScale = std::min(thrustScale * (1.0f + targetFrameDurationFactor), 3.0f);

				glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				return []() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); };
			});

		decoration.renderingSetup = Globals::Components().renderingSetups().size() - 1;
		decoration.renderLayer = RenderLayer::FarMidground;

		return { missile.getComponentId(), decoration.getComponentId(), referenceVelocity, planeId };
	}

	void CreateExplosion(ExplosionParams params)
	{
		auto& particlesShaders = Globals::Shaders().particles();

		Globals::Components().deferredActions().emplace([=, &particlesShaders](float) {
			auto& shockwave = Globals::Components().shockwaves().emplace(params.center_, params.numOfParticles_, params.initVelocity_,
				params.particlesRadius_, params.particlesDensity_, params.particlesLinearDamping_, params.particlesAsBullets_);
			auto& explosionDecoration = Globals::Components().dynamicDecorations().emplace();
			explosionDecoration.customShadersProgram = particlesShaders.getProgramId();
			explosionDecoration.resolutionMode = params.resolutionMode_;
			explosionDecoration.drawMode = GL_POINTS;
			explosionDecoration.bufferDataUsage = GL_DYNAMIC_DRAW;

			Globals::Components().renderingSetups().emplace([=, startTime = Globals::Components().physics().simulationDuration,
				&particlesShaders](Shaders::ProgramId program) mutable
				{
					particlesShaders.vp(Globals::Components().mvp().getVP());
					particlesShaders.texture1(params.explosionTexture_);

					const float elapsed = Globals::Components().physics().simulationDuration - startTime;
					particlesShaders.color(glm::vec4(glm::vec3(glm::pow(1.0f - elapsed / (params.explosionDuration_ * 2.0f), 10.0f)), 1.0f));

					glBlendFunc(GL_SRC_ALPHA, GL_ONE);

					return []() { glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); };
				});

			explosionDecoration.renderingSetup = Globals::Components().renderingSetups().size() - 1;
			explosionDecoration.renderLayer = RenderLayer::FarForeground;

			Globals::Components().deferredActions().emplace([=, startTime = Globals::Components().physics().simulationDuration, &shockwave, &explosionDecoration](float) {
				const float elapsed = Globals::Components().physics().simulationDuration - startTime;
				const float scale = 1.0f + elapsed * 20.0f;

				if (elapsed > params.explosionDuration_)
				{
					shockwave.state = ComponentState::Outdated;
					explosionDecoration.state = ComponentState::Outdated;
					return false;
				}

				explosionDecoration.vertices.clear();
				explosionDecoration.vertices.emplace_back(shockwave.center, scale);
				for (size_t i = 0; i < shockwave.particles.size(); ++i)
				{
					if (i % params.particlesPerDecoration_ != 0) continue;
					const auto& particle = shockwave.particles[i];
					const glm::vec2 position = shockwave.center + (ToVec2<glm::vec2>(particle->GetWorldCenter()) - shockwave.center) * 0.5f;
					explosionDecoration.vertices.emplace_back(position, scale);
				}
				explosionDecoration.state = ComponentState::Changed;

				return true;
			});

			return false;
			});
	}

	void CreateFogForeground(int numOfLayers, float alphaPerLayer, unsigned fogTexture,
		std::function<glm::vec4()> fColor)
	{
		for (int layer = 0; layer < numOfLayers; ++layer)
		for (int posYI = -1; posYI <= 1; ++posYI)
		for (int posXI = -1; posXI <= 1; ++posXI)
		{
			Globals::Components().renderingSetups().emplace([=, texturedProgram = Shaders::Programs::TexturedAccessor()
			](Shaders::ProgramId program) mutable {
				if (!texturedProgram.isValid()) texturedProgram = program;
				texturedProgram.vp(glm::translate(glm::scale(Globals::Components().mvp().getVP(), glm::vec3(glm::vec2(100.0f), 0.0f)),
					glm::vec3(-Globals::Components().camera().prevPosition * (0.002f + layer * 0.002f), 0.0f)));
				texturedProgram.color(fColor()* glm::vec4(1.0f, 1.0f, 1.0f, alphaPerLayer));

				glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				return [texturedProgram]() mutable {
					texturedProgram.vp(Globals::Components().mvp().getVP());
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				};
			});

			Globals::Components().decorations().emplace(Tools::CreateVerticesOfRectangle({ posXI, posYI }, glm::vec2(2.0f, 2.0f) + (layer * 0.2f)),
				TCM::Texture(fogTexture), Tools::CreateTexCoordOfRectangle(), Globals::Components().renderingSetups().size() - 1).renderLayer = RenderLayer::Foreground;
			Globals::Components().decorations().last().resolutionMode = ResolutionMode::LowestLinearBlend1;
		}
	}

	void CreateJuliaBackground(std::function<glm::vec2()> juliaCOffset)
	{
		auto& juliaShaders = Globals::Shaders().julia();
		auto& background = Globals::Components().decorations().emplace(Tools::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 10.0f, 10.0f }));
		background.customShadersProgram = juliaShaders.getProgramId();

		Globals::Components().renderingSetups().emplace([=, &juliaShaders
			](auto) mutable {
				juliaShaders.vp(glm::translate(glm::scale(glm::mat4(1.0f),
					glm::vec3((float)Globals::Components().screenInfo().windowSize.y / Globals::Components().screenInfo().windowSize.x, 1.0f, 1.0f) * 1.5f),
					glm::vec3(-Globals::Components().camera().prevPosition * 0.005f, 0.0f)));
				juliaShaders.juliaCOffset(juliaCOffset());
				juliaShaders.minColor({ 0.0f, 0.0f, 0.0f, 1.0f });
				juliaShaders.maxColor({ 0, 0.1f, 0.2f, 1.0f });
				return nullptr;
			});

		background.renderingSetup = Globals::Components().renderingSetups().size() - 1;
		background.renderLayer = RenderLayer::Background;
		background.resolutionMode = ResolutionMode::LowerLinearBlend1;
	}
}
