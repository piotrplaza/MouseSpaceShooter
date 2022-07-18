#include "actors.hpp"

#include <components/plane.hpp>
#include <components/physics.hpp>
#include <components/mvp.hpp>
#include <components/grapple.hpp>
#include <components/graphicsSettings.hpp>
#include <components/decoration.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <tools/buffersHelpers.hpp>

#include <ogl/shaders/basic.hpp>

namespace
{
	constexpr float planeForwardForce = 10.0f;
	constexpr float mouseSensitivity = 0.01f;
}

namespace Systems
{
	Actors::Actors() = default;

	void Actors::postInit()
	{
		updateStaticBuffers();
	}

	void Actors::step()
	{
		for(auto& plane: Globals::Components().planes())
		{
			if (plane.details.connectedGrappleId && !Globals::Components().grapples().count(plane.details.connectedGrappleId))
			{
				plane.details.grappleJoint.release();
				plane.details.connectedGrappleId = 0;
			}

			if (plane.details.weakConnectedGrappleId && !Globals::Components().grapples().count(plane.details.weakConnectedGrappleId))
				plane.details.weakConnectedGrappleId = 0;

			turn(plane);
			throttle(plane);
			magneticHook(plane);

			connections.updateBuffers();
		}
	}

	void Actors::updateStaticBuffers()
	{
		Tools::UpdateStaticBuffers(Globals::Components().planes(), loadedStaticPlanes);
		loadedStaticPlanes = Globals::Components().planes().size();
	}

	void Actors::turn(Components::Plane& plane) const
	{
		if (glm::length(plane.controls.turningDelta) > 0)
		{
			const float planeAngle = plane.body->GetAngle();
			const float planeSideAngle = planeAngle + glm::half_pi<float>();
			const glm::vec2 planeDirection = { std::cos(planeSideAngle), std::sin(planeSideAngle) };
			const float controllerDot = glm::dot(planeDirection, plane.controls.turningDelta);

			plane.body->SetTransform(plane.body->GetPosition(), planeAngle + controllerDot * mouseSensitivity);
		}

		if (plane.details.grappleJoint && plane.controls.autoRotation)
		{
			const auto& grapple = Globals::Components().grapples()[plane.details.connectedGrappleId];
			const glm::vec2 stepVelocity = (plane.getCenter() - plane.details.previousCenter) - (grapple.getCenter() - grapple.details.previousCenter);
			const float stepVelocityLength = glm::length(stepVelocity);

			if (stepVelocityLength > 0.0f)
			{
				const glm::vec2 normalizedStepVelocity = stepVelocity / stepVelocityLength;

				const float planeAngle = plane.body->GetAngle();
				const glm::vec2 planeDirection = { std::cos(planeAngle), std::sin(planeAngle) };
				const float velocityDot = glm::dot(planeDirection, normalizedStepVelocity);

				const float planeFrontAngle = planeAngle + glm::half_pi<float>();
				const glm::vec2 planeFrontDirection = { std::cos(planeFrontAngle), std::sin(planeFrontAngle) };
				const float velocityFrontDot = glm::dot(planeFrontDirection, normalizedStepVelocity);

				const float planeBackAngle = planeAngle - glm::half_pi<float>();
				const glm::vec2 planeBackDirection = { std::cos(planeBackAngle), std::sin(planeBackAngle) };
				const float velocityBackDot = glm::dot(planeBackDirection, normalizedStepVelocity);

				plane.body->SetTransform(plane.body->GetPosition(), planeAngle + (velocityDot > 0.0f ? velocityFrontDot : velocityBackDot) *
					stepVelocityLength * plane.controls.autoRotationFactor);
			}
		}
	}

	void Actors::throttle(Components::Plane& plane) const
	{
		if (!plane.controls.throttling) return;

		const float currentAngle = plane.body->GetAngle();
		plane.body->ApplyForce(b2Vec2(glm::cos(currentAngle),
			glm::sin(currentAngle)) * planeForwardForce, plane.body->GetWorldCenter(), true);
	}

	void Actors::magneticHook(Components::Plane& plane)
	{
		connections.params.clear();

		ComponentId nearestGrappleId = 0;
		float nearestGrappleDistance = std::numeric_limits<float>::infinity();
		std::vector<ComponentId> grapplesInRange;

		for (const auto& [id, grapple]: Globals::Components().grapples())
		{
			const float grappleDistance = glm::distance(plane.getCenter(), grapple.getCenter());

			if (grappleDistance > grapple.influenceRadius) continue;

			grapplesInRange.push_back(id);

			if (grappleDistance < nearestGrappleDistance)
			{
				nearestGrappleDistance = grappleDistance;
				nearestGrappleId = id;
			}
		}

		if (!plane.controls.magneticHook)
		{
			plane.details.grappleJoint.reset();
			plane.details.connectedGrappleId = 0;
			plane.details.weakConnectedGrappleId = 0;
		}

		for (ComponentId grappleInRange : grapplesInRange)
		{
			const auto& grapple = Globals::Components().grapples()[grappleInRange];

			if (grappleInRange == nearestGrappleId)
			{
				if (plane.controls.magneticHook && !plane.details.grappleJoint &&
					(glm::distance(plane.getCenter(), grapple.getCenter()) >=
					glm::distance(plane.details.previousCenter, grapple.details.previousCenter) ||
					plane.connectIfApproaching))
				{
					plane.details.connectedGrappleId = grappleInRange;
					plane.details.weakConnectedGrappleId = 0;
					createGrappleJoint(plane);
				}
				else if(plane.details.connectedGrappleId != grappleInRange)
				{
					if (plane.controls.magneticHook)
					{
						if (plane.details.grappleJoint)
						{
							connections.params.emplace_back(plane.getCenter(), grapple.getCenter(),
								glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) * 0.2f, 1);
						}
						plane.details.weakConnectedGrappleId = grappleInRange;
					}
					else
					{
						connections.params.emplace_back(plane.getCenter(), grapple.getCenter(),
							glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) * 0.2f, 1);
					}
				}
			}
			else if (plane.details.connectedGrappleId != grappleInRange)
			{
				connections.params.emplace_back(plane.getCenter(), grapple.getCenter(), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) * 0.2f, 1);
			}
		}

		if (plane.details.connectedGrappleId)
		{
			connections.params.emplace_back(plane.getCenter(), Globals::Components().grapples()[plane.details.connectedGrappleId].getCenter(),
				glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) * 0.7f, 20, 0.4f);
		}
		else if (plane.details.weakConnectedGrappleId)
		{
			connections.params.emplace_back(plane.getCenter(), Globals::Components().grapples()[plane.details.weakConnectedGrappleId].getCenter(),
				glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) * 0.5f, 1);
		}
	}

	void Actors::createGrappleJoint(Components::Plane& plane) const
	{
		assert(plane.details.connectedGrappleId);

		const auto& grapple = Globals::Components().grapples()[plane.details.connectedGrappleId];

		b2DistanceJointDef distanceJointDef;
		distanceJointDef.bodyA = plane.body.get();
		distanceJointDef.bodyB = grapple.body.get();
		distanceJointDef.localAnchorA = distanceJointDef.bodyA->GetLocalCenter();
		distanceJointDef.localAnchorB = distanceJointDef.bodyB->GetLocalCenter();
		distanceJointDef.length = glm::distance(plane.getCenter(), grapple.getCenter());
		distanceJointDef.minLength = distanceJointDef.length;
		distanceJointDef.maxLength = distanceJointDef.length;
		distanceJointDef.collideConnected = true;
		plane.details.grappleJoint.reset(Globals::Components().physics().world->CreateJoint(&distanceJointDef));
	}

	std::vector<glm::vec3> Actors::Connections::Params::getVertices() const
	{
		if (segmentsNum == 1) return { { p1, 0.0f }, { p2, 0.0f } };
		else return Tools::CreateVerticesOfLightning(p1, p2, segmentsNum, frayFactor);
	}

	std::vector<glm::vec4> Actors::Connections::Params::getColors() const
	{
		return std::vector<glm::vec4>(segmentsNum * 2, color);
	}

	Actors::Connections::Connections()
	{
		auto& decoration = EmplaceDynamicComponent(Globals::Components().dynamicDecorations(), {});
		decoration.drawMode = GL_LINES;
		decoration.bufferDataUsage = GL_DYNAMIC_DRAW;
		decoration.renderLayer = RenderLayer::FarMidground;
		decorationId = decoration.getComponentId();
	}

	void Actors::Connections::updateBuffers()
	{
		auto& connectionsDecoration = Globals::Components().dynamicDecorations()[decorationId];

		connectionsDecoration.vertices.clear();
		connectionsDecoration.colors.clear();

		for (auto& connectionParams : params)
		{
			if (connectionParams.segmentsNum > 1)
				connectionParams.segmentsNum = std::max((int)glm::distance(connectionParams.p1, connectionParams.p2) * 2, 2);

			const auto vertices = connectionParams.getVertices();
			connectionsDecoration.vertices.insert(connectionsDecoration.vertices.end(), vertices.begin(), vertices.end());

			const auto colors = connectionParams.getColors();
			connectionsDecoration.colors.insert(connectionsDecoration.colors.end(), colors.begin(), colors.end());
		}

		assert(connectionsDecoration.vertices.size() == connectionsDecoration.colors.size());

		connectionsDecoration.state = ComponentState::Changed;
	}
}
