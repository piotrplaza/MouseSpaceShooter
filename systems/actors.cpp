#include "actors.hpp"

#include <components/plane.hpp>
#include <components/physics.hpp>
#include <components/mvp.hpp>
#include <components/grapple.hpp>
#include <components/graphicsSettings.hpp>
#include <components/decoration.hpp>
#include <components/functor.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <tools/buffersHelpers.hpp>
#include <tools/b2Helpers.hpp>
#include <tools/shapes2D.hpp>

#include <ogl/shaders/textured.hpp>

namespace
{
	constexpr float planeForwardForce = 10.0f;
}

namespace Systems
{
	Actors::Actors() = default;

	void Actors::postInit()
	{
		auto& planes = Globals::Components().planes();
		auto& grapples = Globals::Components().grapples();

		auto prevCenterUpdate = [&]() {
			for (auto& plane : planes)
				plane.details.previousCenter = plane.getOrigin2D();

			for (auto& grapple : grapples)
				grapple.details.previousCenter = grapple.getOrigin2D();

			return true;
		};

		prevCenterUpdate();
		Globals::Components().stepTeardowns().emplace(prevCenterUpdate);
		Globals::Components().renderSetups().emplace([&]() {
			Globals::Shaders().textured().numOfPlayers(planes.size());
			unsigned playersCounter = 0;
			for (const auto& plane : planes)
				Globals::Shaders().textured().playersCenter(playersCounter++, plane.getOrigin2D());

			return true;
		});
	}

	void Actors::step()
	{
		auto& planes = Globals::Components().planes();
		auto& grapples = Globals::Components().grapples();

		std::erase_if(allConnections, [&](const auto& element) {
			return !planes.contains(element.first);
			});

		for (auto& plane : planes)
		{
			auto& planeConnections = allConnections[plane.getComponentId()];

			if (!plane.isEnabled())
			{
				planeConnections.params.clear();
				planeConnections.updateBuffers();
				continue;
			}

			if (plane.details.connectedGrappleId && !Globals::Components().grapples().contains(*plane.details.connectedGrappleId))
			{
				plane.details.grappleJoint.release();
				plane.details.connectedGrappleId = std::nullopt;
			}

			if (plane.details.weakConnectedGrappleId && !Globals::Components().grapples().contains(*plane.details.weakConnectedGrappleId))
				plane.details.weakConnectedGrappleId = std::nullopt;

			turn(plane);
			throttle(plane);
			magneticHook(plane, planeConnections);

			planeConnections.updateBuffers();

			plane.step();
		}

		updateDynamicBuffers();
	}

	void Actors::updateDynamicBuffers()
	{
		Tools::UpdateDynamicBuffers(Globals::Components().planes());
	}

	void Actors::turn(Components::Plane& plane) const
	{
		const auto& physics = Globals::Components().physics();

		const float turningDeltaLength = glm::length(plane.controls.turningDelta);
		const glm::vec2 normalizedTurningDelta = plane.controls.turningDelta / turningDeltaLength;
		const float targetTurningDeltaLength = std::min(turningDeltaLength, 1.0f);

		if (plane.details.grappleJoint && plane.controls.autoRotation)
		{
			const auto& grapple = Globals::Components().grapples()[*plane.details.connectedGrappleId];
			const glm::vec2 stepVelocity = (plane.getOrigin2D() - plane.details.previousCenter) - (grapple.getOrigin2D() - grapple.details.previousCenter);
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
		else if (targetTurningDeltaLength > 0)
		{
			const float planeAngle = plane.body->GetAngle();
			const float planeSideAngle = planeAngle + glm::half_pi<float>();
			const glm::vec2 planeDirection = { std::cos(planeSideAngle), std::sin(planeSideAngle) };
			const float controllerDot = glm::dot(planeDirection, normalizedTurningDelta);

			plane.body->SetTransform(plane.body->GetPosition(), planeAngle + controllerDot * targetTurningDeltaLength);
		}
	}

	void Actors::throttle(Components::Plane& plane) const
	{
		plane.throttle(plane.controls.throttling * planeForwardForce);
	}

	void Actors::magneticHook(Components::Plane& plane, Connections& planeConnections)
	{
		planeConnections.params.clear();

		ComponentId nearestGrappleId = 0;
		float nearestGrappleDistance = std::numeric_limits<float>::infinity();
		std::vector<ComponentId> grapplesInRange;

		for (const auto& grapple: Globals::Components().grapples())
		{
			const float grappleDistance = glm::distance(plane.getOrigin2D(), grapple.getOrigin2D());

			if (grappleDistance > grapple.influenceRadius) continue;

			grapplesInRange.push_back(grapple.getComponentId());

			if (grappleDistance < nearestGrappleDistance)
			{
				nearestGrappleDistance = grappleDistance;
				nearestGrappleId = grapple.getComponentId();
			}
		}

		if (!plane.controls.magneticHook)
		{
			plane.details.grappleJoint.reset();
			plane.details.connectedGrappleId = std::nullopt;
			plane.details.weakConnectedGrappleId = std::nullopt;
		}

		for (ComponentId grappleInRange : grapplesInRange)
		{
			const auto& grapple = Globals::Components().grapples()[grappleInRange];

			if (grappleInRange == nearestGrappleId)
			{
				if (plane.controls.magneticHook && !plane.details.grappleJoint &&
					(glm::distance(plane.getOrigin2D(), grapple.getOrigin2D()) >=
					glm::distance(plane.details.previousCenter, grapple.details.previousCenter) ||
					plane.connectIfApproaching))
				{
					plane.details.connectedGrappleId = grappleInRange;
					plane.details.weakConnectedGrappleId = std::nullopt;
					createGrappleJoint(plane);
				}
				else if(plane.details.connectedGrappleId != grappleInRange)
				{
					if (plane.controls.magneticHook)
					{
						if (plane.details.grappleJoint)
						{
							planeConnections.params.emplace_back(plane.getOrigin2D(), grapple.getOrigin2D(),
								glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) * 0.2f, 1);
						}
						plane.details.weakConnectedGrappleId = grappleInRange;
					}
					else
					{
						planeConnections.params.emplace_back(plane.getOrigin2D(), grapple.getOrigin2D(),
							glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) * 0.2f, 1);
					}
				}
			}
			else if (plane.details.connectedGrappleId != grappleInRange)
			{
				planeConnections.params.emplace_back(plane.getOrigin2D(), grapple.getOrigin2D(), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) * 0.2f, 1);
			}
		}

		if (plane.details.connectedGrappleId)
		{
			planeConnections.params.emplace_back(plane.getOrigin2D(), Globals::Components().grapples()[*plane.details.connectedGrappleId].getOrigin2D(),
				glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) * 0.7f, 20, 0.4f);
		}
		else if (plane.details.weakConnectedGrappleId)
		{
			planeConnections.params.emplace_back(plane.getOrigin2D(), Globals::Components().grapples()[*plane.details.weakConnectedGrappleId].getOrigin2D(),
				glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) * 0.5f, 1);
		}
	}

	void Actors::createGrappleJoint(Components::Plane& plane) const
	{
		assert(plane.details.connectedGrappleId);

		const auto& grapple = Globals::Components().grapples()[*plane.details.connectedGrappleId];

		plane.details.grappleJoint.reset(Tools::CreateDistanceJoint(*plane.body, *grapple.body,
			ToVec2<glm::vec2>(plane.body->GetWorldCenter()), ToVec2<glm::vec2>(grapple.body->GetWorldCenter()),
			true, glm::distance(plane.getOrigin2D(), grapple.getOrigin2D())));
	}

	std::vector<glm::vec3> Actors::Connections::Params::getVertices() const
	{
		if (segmentsNum == 1) return { { p1, 0.0f }, { p2, 0.0f } };
		else return Shapes2D::CreateVerticesOfLightning(p1, p2, segmentsNum, frayFactor);
	}

	std::vector<glm::vec4> Actors::Connections::Params::getColors() const
	{
		return std::vector<glm::vec4>(segmentsNum * 2, color);
	}

	Actors::Connections::Connections()
	{
		auto& decoration = Globals::Components().dynamicDecorations().emplace();
		decoration.drawMode = GL_LINES;
		decoration.bufferDataUsage = GL_DYNAMIC_DRAW;
		decoration.renderLayer = RenderLayer::FarMidground;
		decorationId = decoration.getComponentId();
	}

	Actors::Connections::~Connections()
	{
		auto& decoration = Globals::Components().dynamicDecorations()[decorationId];
		decoration.state = ComponentState::Outdated;
	}

	void Actors::Connections::updateBuffers()
	{
		auto& decoration = Globals::Components().dynamicDecorations()[decorationId];

		decoration.vertices.clear();
		decoration.colors.clear();

		for (auto& connectionParams : params)
		{
			if (connectionParams.segmentsNum > 1)
				connectionParams.segmentsNum = std::max((int)glm::distance(connectionParams.p1, connectionParams.p2) * 2, 2);

			const auto vertices = connectionParams.getVertices();
			decoration.vertices.insert(decoration.vertices.end(), vertices.begin(), vertices.end());

			const auto colors = connectionParams.getColors();
			decoration.colors.insert(decoration.colors.end(), colors.begin(), colors.end());
		}

		assert(decoration.vertices.size() == decoration.colors.size());

		decoration.state = ComponentState::Changed;
	}
}
