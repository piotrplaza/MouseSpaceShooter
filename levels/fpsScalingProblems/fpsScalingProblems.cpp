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
			auto& walls = Globals::Components().walls();

			walls.emplace_back(Tools::CreateCircleBody({ 0.0f, 0.0f }, 1.0f));
			walls.emplace_back(Tools::CreateCircleBody({ 0.0f, 5.0f }, 1.0f, b2_dynamicBody));

			Tools::CreateDistanceJoint(*walls[0].body, *walls[1].body, walls[0].getCenter(), walls[1].getCenter(), true, distance(walls[0].getCenter(), walls[1].getCenter()));
		}

		void step()
		{
			const float force = 20.0f;

			auto& walls = Globals::Components().walls();
			auto& physics = Globals::Components().physics();

			const glm::vec2 fromCenterVec = glm::normalize(walls[1].getCenter() - walls[0].getCenter());
			const glm::vec2 forceVec = rotate(fromCenterVec, -glm::half_pi<float>());
			walls[1].body->ApplyForceToCenter(ToVec2<b2Vec2>(forceVec * force), false);
			
			const float v = length(walls[1].getVelocity());

			if (timeToSpecificVelocity == 0.0f && v >= 26.0f)
			{
				timeToSpecificVelocity = physics.simulationDuration;
			}

			std::cout << length(walls[1].getVelocity()) << " " << timeToSpecificVelocity << std::endl;
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
