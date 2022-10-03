#pragma once

#include <commonTypes/typeComponentMappers.hpp>
#include <commonTypes/resolutionMode.hpp>

#include <ogl/shaders/particles.hpp>

#include <glm/vec2.hpp>

#include <functional>
#include <optional>

class b2Body;

namespace Shaders
{
	namespace Programs
	{
		struct Julia;
	}
}

namespace Tools
{
	struct ExplosionParams
	{
		ExplosionParams& center(glm::vec2 value)
		{
			center_ = value;
			return *this;
		}

		ExplosionParams& sourceVelocity(glm::vec2 value)
		{
			sourceVelocity_ = value;
			return *this;
		}

		ExplosionParams& numOfParticles(int value)
		{
			numOfParticles_ = value;
			return *this;
		}

		ExplosionParams& initExplosionVelocity(float value)
		{
			initExplosionVelocity_ = value;
			return *this;
		}

		ExplosionParams& initExplosionVelocityRandomMinFactor(float value)
		{
			initExplosionVelocityRandomMinFactor_ = value;
			return *this;
		}

		ExplosionParams& particlesRadius(float value)
		{
			particlesRadius_ = value;
			return *this;
		}

		ExplosionParams& particlesDensity(float value)
		{
			particlesDensity_ = value;
			return *this;
		}

		ExplosionParams& particlesLinearDamping(float value)
		{
			particlesLinearDamping_ = value;
			return *this;
		}

		ExplosionParams& particlesAsBullets(bool value)
		{
			particlesAsBullets_ = value;
			return *this;
		}

		ExplosionParams& explosionDuration(float value)
		{
			explosionDuration_ = value;
			return *this;
		}

		ExplosionParams& explosionTexture(unsigned value)
		{
			explosionTexture_ = value;
			return *this;
		}

		ExplosionParams& particlesPerDecoration(int value)
		{
			particlesPerDecoration_ = value;
			return *this;
		}

		ExplosionParams& resolutionMode(ResolutionMode value)
		{
			resolutionMode_ = value;
			return *this;
		}

		glm::vec2 center_ = { 0.0f, 0.0f };
		glm::vec2 sourceVelocity_ = { 0.0f, 0.0f };
		int numOfParticles_ = 64;
		float initExplosionVelocity_ = 100.0f;
		float initExplosionVelocityRandomMinFactor_ = 0.5f;
		float particlesRadius_ = 1.0f;
		float particlesDensity_ = 0.01f;
		float particlesLinearDamping_ = 3.0f;
		bool particlesAsBullets_ = false;
		float explosionDuration_ = 1.0f;
		unsigned explosionTexture_ = 0;
		int particlesPerDecoration_ = 4;
		ResolutionMode resolutionMode_ = ResolutionMode::Normal;
	};

	struct PlayerHandler
	{
		unsigned playerId = 0;
		std::optional<unsigned> gamepadId;
		float durationToLaunchMissile = 0.0f;
	};

	class PlayersHandler
	{
	public:
		void initPlayers(unsigned rocketPlaneTexture, const std::array<unsigned, 4>& flameAnimatedTextureForPlayers, bool gamepadForPlayer1,
			std::function<glm::vec2(unsigned player, unsigned numOfPlayers)> initPosF);
		void initMultiplayerCamera(std::function<float()> projectionHSizeMin, float scalingFactor = 0.6f, float velocityFactor = 0.2f, float transitionFactor = 10.0f) const;

		void autodetectionStep(std::function<glm::vec2(unsigned player)> initPosF);
		void controlStep(std::function<void(unsigned playerHandlerId, bool fire)> fireF = nullptr) const;

		const std::vector<Tools::PlayerHandler>& getPlayersHandlers() const;
		std::vector<Tools::PlayerHandler>& accessPlayersHandlers();

	private:
		std::vector<Tools::PlayerHandler> playersHandlers;

		unsigned rocketPlaneTexture = 0;
		std::array<unsigned, 4> flameAnimatedTextureForPlayers{ 0 };
		bool gamepadForPlayer1 = false;
	};

	struct MissileHandler
	{
		MissileHandler();
		MissileHandler(ComponentId missileId, ComponentId backThrustId, glm::vec2 referenceVelocity, std::optional<ComponentId> planeId = std::nullopt);
		MissileHandler(MissileHandler&& other) noexcept;

		~MissileHandler();

		MissileHandler& operator=(MissileHandler&& other) noexcept;

		ComponentId missileId = 0;
		ComponentId backThrustId = 0;
		glm::vec2 referenceVelocity{};
		std::optional<ComponentId> planeId;

	private:
		bool valid = true;
	};

	class MissilesHandler
	{
	public:
		MissilesHandler();

		void setPlayersHandler(Tools::PlayersHandler& playersHandler);
		void setExplosionTexture(unsigned explosionTexture);
		void setMissileTexture(unsigned missileTexture);
		void setFlameAnimatedTexture(unsigned flameAnimatedTexture);

		void setExplosionParams(Tools::ExplosionParams explosionParams);

		void setResolutionModeF(std::function<ResolutionMode(const b2Body&)> resolutionModeF);
		void setExplosionF(std::function<void()> explosionF);

		void launchingMissile(unsigned playerHandlerId, bool tryToLaunch);

	private:
		void launchMissile(unsigned playerId);

		std::unordered_map<ComponentId, Tools::MissileHandler> missilesToHandlers;

		Tools::PlayersHandler* playersHandler = nullptr;

		unsigned explosionTexture = 0;
		unsigned missileTexture = 0;
		unsigned flameAnimatedTexture = 0;

		Tools::ExplosionParams explosionParams;

		std::function<ResolutionMode(const b2Body&)> resolutionModeF;
		std::function<void()> explosionF;
	};

	ComponentId CreatePlane(unsigned planeTexture, unsigned flameAnimatedTexture, glm::vec2 position = glm::vec2(0.0f), float angle = 0.0f);
	MissileHandler CreateMissile(glm::vec2 startPosition, float startAngle, float force, glm::vec2 referenceVelocity,
		glm::vec2 initialVelocity, unsigned missileTexture, unsigned flameAnimatedTexture, std::optional<ComponentId> planeId = std::nullopt);
	void CreateExplosion(ExplosionParams params);
	void CreateFogForeground(int numOfLayers, float alphaPerLayer, unsigned fogTexture,
		std::function<glm::vec4()> fColor = []() { return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); });
	void CreateJuliaBackground(std::function<glm::vec2()> juliaCOffset);
}
