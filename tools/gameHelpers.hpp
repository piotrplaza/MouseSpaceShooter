#pragma once

#include "b2Helpers.hpp"

#include <commonTypes/componentMappers.hpp>
#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/componentId.hpp>
#include <commonTypes/fTypes.hpp>
#include <commonTypes/renderLayer.hpp>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <functional>
#include <optional>
#include <list>
#include <memory>

class b2Body;
class b2Fixture;

namespace ShadersUtils
{
	namespace Programs
	{
		struct Julia;
	}
}

namespace Components
{
	struct Sound;
	struct Shockwave;
}

namespace Tools
{
	struct PlaneParams
	{
		PlaneParams& position(glm::vec2 value)
		{
			position_ = value;
			return *this;
		}

		PlaneParams& angle(float value)
		{
			angle_ = value;
			return *this;
		}

		PlaneParams& numOfThrusts(int value)
		{
			numOfThrusts_ = value;
			return *this;
		}

		PlaneParams& thrustAngle(float value)
		{
			thrustAngle_ = value;
			return *this;
		}

		PlaneParams& thrustOffset(glm::vec2 value)
		{
			thrustOffset_ = value;
			return *this;
		}

		PlaneParams& collisionBoxRendering(bool value)
		{
			collisionBoxRendering_ = value;
			return *this;
		}

		glm::vec2 position_ = { 0.0f, 0.0f };
		float angle_ = 0.0f;
		int numOfThrusts_ = 2;
		float thrustAngle_ = 0.1f;
		glm::vec2 thrustOffset_ = { -0.5f, 0.4f };
		bool collisionBoxRendering_ = false;
	};

	struct JuliaParams
	{
		JuliaParams& juliaCF(FVec2 value)
		{
			juliaCF_ = std::move(value);
			return *this;
		}

		JuliaParams& juliaCOffsetF(FVec2 value)
		{
			juliaCOffsetF_ = std::move(value);
			return *this;
		}

		JuliaParams& minColorF(FVec4 value)
		{
			minColorF_ = std::move(value);
			return *this;
		}

		JuliaParams& maxColorF(FVec4 value)
		{
			maxColorF_ = std::move(value);
			return *this;
		}

		JuliaParams& iterationsF(FInt value)
		{
			iterationsF_ = std::move(value);
			return *this;
		}

		FVec2 juliaCF_ = glm::vec2{ -0.1f, 0.65f };
		FVec2 juliaCOffsetF_ = glm::vec2{ 0.0f };
		FVec4 minColorF_ = glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
		FVec4 maxColorF_ = glm::vec4{ 0.0f, 0.2f, 0.1f, 1.0f };
		FInt iterationsF_ = 100;
	};

	struct MissileHandler
	{
		MissileHandler();
		MissileHandler(CM::Missile missile, CM::Decoration backThrust, glm::vec2 referenceVelocity, std::optional<CM::Plane> plane = std::nullopt,
			std::optional<CM::Sound> sound = std::nullopt);
		MissileHandler(MissileHandler&& other) noexcept;

		~MissileHandler();

		MissileHandler& operator=(MissileHandler&& other) noexcept;

		CM::Missile missile;
		CM::Decoration backThrust;
		glm::vec2 referenceVelocity{};
		std::optional<CM::Plane> plane;
		std::optional<CM::Sound> sound;
		bool collided = false;

	private:
		bool valid = true;
	};

	class SoundsLimitter: public std::enable_shared_from_this<SoundsLimitter>
	{
	public:
		static std::shared_ptr<SoundsLimitter> create(unsigned limit);

		Components::Sound& newSound(Components::Sound& sound, std::function<void()> tearDown = nullptr);

	private:
		SoundsLimitter(unsigned limit);

		const unsigned limit;
		std::list<Components::Sound*> sounds;
	};

	const Tools::BodyParams& GetDefaultParamsForPlaneBody();
	ComponentId CreatePlane(Body body, CM::Texture planeTexture, CM::AnimatedTexture thrustAnimatedTexture, PlaneParams params = PlaneParams());
	MissileHandler CreateMissile(glm::vec2 startPosition, float startAngle, float force, glm::vec2 referenceVelocity,
		glm::vec2 initialVelocity, CM::Texture missileTexture, CM::AnimatedTexture thrustAnimatedTexture, std::optional<CM::Plane> planeId = std::nullopt,
		std::optional<CM::SoundBuffer> missileSoundBuffer = std::nullopt);
	void CreateFogForeground(int numOfLayers, float alphaPerLayer, CM::Texture fogTexture, FVec4 fColor = glm::vec4(1.0f), std::function<glm::vec2(int layer)> textureTranslation = nullptr);
	void CreateJuliaBackground(JuliaParams params);
	Components::Sound& CreateAndPlaySound(CM::SoundBuffer soundBuffer, FVec2 posF = nullptr,
		std::function<void(Components::Sound&)> config = nullptr, std::function<void(Components::Sound&)> stepF = nullptr);
	std::function<void(b2Fixture&, b2Fixture&)> SkipDuplicatedBodiesCollisions(std::function<void(b2Fixture&, b2Fixture&)> handler);
	void CountDown(CM::AnimatedTexture digitsAnimatedTexture, CM::SoundBuffer countSoundBufferId, CM::SoundBuffer startSoundBufferId, float onScreenScale = 0.2f, std::function<void()> startF = nullptr);
}
