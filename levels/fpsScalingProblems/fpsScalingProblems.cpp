#include "fpsScalingProblems.hpp"

#include <components/wall.hpp>
#include <components/physics.hpp>

#include <globals/components.hpp>

#include <tools/b2Helpers.hpp>

#include <glm/gtx/rotate_vector.hpp>

#include <iostream>

namespace Levels
{
	class FPSScalingProblems::Impl
	{
	public:
		void createWalls()
		{
			auto& staticWalls = Globals::Components().staticWalls();

			auto& wall1 = staticWalls.emplace(Tools::CreateCircleBody(1.0f, Tools::BodyParams()));
			auto& wall2 = staticWalls.emplace(Tools::CreateCircleBody(1.0f, Tools::BodyParams().position({ 0.0f, 5.0f }).bodyType(b2_dynamicBody)));

			Tools::CreateDistanceJoint(*wall1.body, *wall2.body, wall1.getOrigin(), wall2.getOrigin(), true, distance(wall1.getOrigin(), wall2.getOrigin()));
		}

		void step()
		{
			const float force = 20.0f;

			auto& staticWalls = Globals::Components().staticWalls();
			auto& physics = Globals::Components().physics();

			const glm::vec2 fromCenterVec = glm::normalize(staticWalls[1].getOrigin() - staticWalls[0].getOrigin());
			const glm::vec2 forceVec = rotate(fromCenterVec, -glm::half_pi<float>());
			staticWalls[1].body->ApplyForceToCenter(ToVec2<b2Vec2>(forceVec * force), false);
			
			const float v = length(staticWalls[1].getVelocity());

			if (timeToSpecificVelocity == 0.0f && v >= 26.0f)
			{
				timeToSpecificVelocity = physics.simulationDuration;
			}

			std::cout << length(staticWalls[1].getVelocity()) << " " << timeToSpecificVelocity << std::endl;
		}

	private:
		float timeToSpecificVelocity = 0.0f;
	};

	FPSScalingProblems::FPSScalingProblems():
		impl(std::make_unique<Impl>())
	{
		impl->createWalls();
	}

	FPSScalingProblems::~FPSScalingProblems() = default;

	void FPSScalingProblems::step()
	{
		impl->step();
	}
}
