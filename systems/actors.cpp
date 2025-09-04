#include "actors.hpp"

#include <components/actor.hpp>
#include <components/plane.hpp>
#include <components/physics.hpp>
#include <components/vp.hpp>
#include <components/grapple.hpp>
#include <components/graphicsSettings.hpp>
#include <components/decoration.hpp>
#include <components/functor.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <tools/buffersHelpers.hpp>
#include <tools/b2Helpers.hpp>
#include <tools/Shapes2D.hpp>

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
	}

	void Actors::teardown()
	{
		allConnections.clear();
	}

	void Actors::step()
	{
		{
			auto& planes = Globals::Components().planes();
			unsigned playersCounter = 0;

			for (auto& plane: planes)
			{
				if (!plane.teardownF)
					plane.teardownF = [&]() { allConnections.erase(plane.getComponentId()); };

				auto& planeConnections = allConnections[plane.getComponentId()];

				if (!plane.isEnabled())
				{
					planeConnections.params.clear();
					planeConnections.updateBuffers();
					continue;
				}

				if (plane.details.connectedGrappleId && ((plane.details.connectedGrappleId->first && !Globals::Components().staticGrapples().contains(plane.details.connectedGrappleId->second))
					|| (!plane.details.connectedGrappleId->first && !Globals::Components().grapples().contains(plane.details.connectedGrappleId->second))))
				{
					plane.details.grappleJoint.release();
					plane.details.connectedGrappleId = std::nullopt;
				}

				if (plane.details.weakConnectedGrappleId && ((plane.details.weakConnectedGrappleId->first && !Globals::Components().staticGrapples().contains(plane.details.weakConnectedGrappleId->second))
					|| (!plane.details.weakConnectedGrappleId->first && !Globals::Components().grapples().contains(plane.details.weakConnectedGrappleId->second))))
					plane.details.weakConnectedGrappleId = std::nullopt;

				turn(plane);
				throttle(plane);
				grappleHook(plane, planeConnections);

				planeConnections.updateBuffers();

				plane.step();
				Globals::Shaders().textured().playersCenter(playersCounter++, plane.getOrigin2D());
				plane.details.previousCenter = plane.getOrigin2D();
			}

			Globals::Shaders().textured().numOfPlayers(playersCounter);

			for (auto& grapple: Globals::Components().staticGrapples())
			{
				grapple.step();
				grapple.details.previousCenter = grapple.getOrigin2D();
			}

			for (auto& grapple: Globals::Components().grapples())
			{
				grapple.step();
				grapple.details.previousCenter = grapple.getOrigin2D();
			}
		}

		{
			auto& actors = Globals::Components().actors();
			for (auto& actor : actors)
				actor.step();
		}

		updateDynamicBuffers();
	}

	void Actors::updateDynamicBuffers()
	{
		Tools::ProcessDynamicRenderableComponents(Globals::Components().actors());
		Tools::ProcessDynamicRenderableComponents(Globals::Components().planes());
	}

	void Actors::turn(Components::Plane& plane) const
	{
		const auto& physics = Globals::Components().physics();

		const float turningDeltaLength = glm::length(plane.controls.turningDelta);
		const glm::vec2 normalizedTurningDelta = plane.controls.turningDelta / turningDeltaLength;
		const float targetTurningDeltaLength = std::min(turningDeltaLength, 1.0f);

		if (plane.details.grappleJoint && plane.controls.autoRotation)
		{
			const auto& grapple = plane.details.connectedGrappleId->first
				? Globals::Components().staticGrapples()[plane.details.connectedGrappleId->second]
				: Globals::Components().grapples()[plane.details.connectedGrappleId->second];
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

	void Actors::grappleHook(Components::Plane& plane, Connections& planeConnections)
	{
		using MPBehavior = Components::Grapple::MPBehavior;
		auto& planes = Globals::Components().planes();
		planeConnections.params.clear();

		ComponentId nearestGrappleId = 0;
		float nearestGrappleDistance = std::numeric_limits<float>::infinity();
		std::vector<std::pair<bool, ComponentId>> grapplesInRange;

		if (!plane.controls.grappleHook)
		{
			plane.details.grappleJoint.reset();
			plane.details.connectedGrappleId = std::nullopt;
			plane.details.weakConnectedGrappleId = std::nullopt;
		}

		auto processGraples = [&](auto& grapples) {
			for (const auto& grapple : grapples)
			{
				std::vector<Components::Plane*> otherConnectedPlanes;

				if (grapple.multiplayerBehavior != MPBehavior::All)
				{
					auto isOtherConnectedPlane = [&](const Components::Plane& p) {
						return p.isEnabled() && p.getComponentId() != plane.getComponentId() && p.details.connectedGrappleId &&
						((p.details.connectedGrappleId->first && grapple.isStatic() && p.details.connectedGrappleId->second == grapple.getComponentId()) ||
							(!p.details.connectedGrappleId->first && !grapple.isStatic() && p.details.connectedGrappleId->second == grapple.getComponentId()));
					};
					for (auto it = std::find_if(planes.begin(), planes.end(), isOtherConnectedPlane); it != planes.end(); it = std::find_if(++it, planes.end(), isOtherConnectedPlane))
						otherConnectedPlanes.push_back(&*it);
				}

				const float grappleDistance = glm::distance(plane.getOrigin2D(), grapple.getOrigin2D());

				if (grappleDistance > grapple.range)
					continue;

				grapplesInRange.emplace_back(grapple.isStatic(), grapple.getComponentId());
				const bool isCurrentPlaneFastest = [&]() {
					auto* fastestPlane = &plane;
					for (auto* otherConnectedPlane : otherConnectedPlanes)
					{
						if (glm::length(otherConnectedPlane->getVelocity()) > glm::length(fastestPlane->getVelocity()))
							fastestPlane = otherConnectedPlane;
					}
					return fastestPlane == &plane;
					}();
				if (grappleDistance < nearestGrappleDistance &&
					(grapple.multiplayerBehavior == MPBehavior::All ||
						(grapple.multiplayerBehavior == MPBehavior::First && otherConnectedPlanes.empty()) ||
						(grapple.multiplayerBehavior == MPBehavior::Fastest && isCurrentPlaneFastest)))
				{
					nearestGrappleDistance = grappleDistance;
					nearestGrappleId = grapple.getComponentId();
				}
				if (grapple.multiplayerBehavior == MPBehavior::Fastest && isCurrentPlaneFastest)
					for (auto* otherConnectedPlane : otherConnectedPlanes)
					{
						otherConnectedPlane->details.grappleJoint.reset();
						otherConnectedPlane->details.connectedGrappleId = std::nullopt;
						otherConnectedPlane->details.weakConnectedGrappleId = std::nullopt;
					}
			}
		};

		processGraples(Globals::Components().staticGrapples());
		processGraples(Globals::Components().grapples());

		for (const auto isStaticAndGrappleId : grapplesInRange)
		{
			const auto& grapple = isStaticAndGrappleId.first
				? Globals::Components().staticGrapples()[isStaticAndGrappleId.second]
				: Globals::Components().grapples()[isStaticAndGrappleId.second];

			if (isStaticAndGrappleId.second == nearestGrappleId)
			{
				if (plane.controls.grappleHook && !plane.details.grappleJoint &&
					(glm::distance(plane.getOrigin2D(), grapple.getOrigin2D()) >=
					glm::distance(plane.details.previousCenter, grapple.details.previousCenter) ||
					grapple.connectIfApproaching))
				{
					plane.details.connectedGrappleId = isStaticAndGrappleId;
					plane.details.weakConnectedGrappleId = std::nullopt;
					createGrappleJoint(plane);
				}
				else if(plane.details.connectedGrappleId != isStaticAndGrappleId)
				{
					if (plane.controls.grappleHook)
					{
						if (plane.details.grappleJoint)
						{
							planeConnections.params.emplace_back(plane.getOrigin2D(), grapple.getOrigin2D(),
								glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) * 0.2f, 1);
						}
						plane.details.weakConnectedGrappleId = isStaticAndGrappleId;
					}
					else
					{
						planeConnections.params.emplace_back(plane.getOrigin2D(), grapple.getOrigin2D(),
							glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) * 0.2f, 1);
					}
				}
			}
			else if (plane.details.connectedGrappleId != isStaticAndGrappleId)
			{
				planeConnections.params.emplace_back(plane.getOrigin2D(), grapple.getOrigin2D(), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) * 0.2f, 1);
			}
		}

		if (plane.details.connectedGrappleId)
		{
			planeConnections.params.emplace_back(plane.getOrigin2D(), plane.details.connectedGrappleId->first
				? Globals::Components().staticGrapples()[plane.details.connectedGrappleId->second].getOrigin2D()
				: Globals::Components().grapples()[plane.details.connectedGrappleId->second].getOrigin2D(),
				glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) * 0.7f, 20, 0.4f);
		}
		else if (plane.details.weakConnectedGrappleId)
		{
			planeConnections.params.emplace_back(plane.getOrigin2D(), plane.details.weakConnectedGrappleId->first
				? Globals::Components().staticGrapples()[plane.details.weakConnectedGrappleId->second].getOrigin2D()
				: Globals::Components().grapples()[plane.details.weakConnectedGrappleId->second].getOrigin2D(),
				glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) * 0.5f, 1);
		}
	}

	void Actors::createGrappleJoint(Components::Plane& plane) const
	{
		assert(plane.details.connectedGrappleId);

		const auto& grapple = plane.details.connectedGrappleId->first
			? Globals::Components().staticGrapples()[plane.details.connectedGrappleId->second]
			: Globals::Components().grapples()[plane.details.connectedGrappleId->second];

		plane.details.grappleJoint.reset(Tools::CreateDistanceJoint(*plane.body, *grapple.body,
			ToVec2<glm::vec2>(plane.body->GetPosition()), ToVec2<glm::vec2>(grapple.body->GetPosition()),
			true, glm::distance(plane.getOrigin2D(), grapple.getOrigin2D())));
	}

	std::vector<glm::vec3> Actors::Connections::Params::getPositions() const
	{
		if (segmentsNum == 1) return { { p1, 0.0f }, { p2, 0.0f } };
		else return Tools::Shapes2D::CreatePositionsOfLightning(p1, p2, segmentsNum, frayFactor);
	}

	std::vector<glm::vec4> Actors::Connections::Params::getColors() const
	{
		return std::vector<glm::vec4>(segmentsNum * 2, color);
	}

	Actors::Connections::Connections()
	{
		auto& decoration = Globals::Components().decorations().emplace();
		decoration.drawMode = GL_LINES;
		decoration.bufferDataUsage = GL_DYNAMIC_DRAW;
		decoration.renderLayer = RenderLayer::FarMidground;
		decorationId = decoration.getComponentId();
	}

	Actors::Connections::~Connections()
	{
		auto& decoration = Globals::Components().decorations()[decorationId];
		decoration.state = ComponentState::Outdated;
	}

	void Actors::Connections::updateBuffers()
	{
		auto& decoration = Globals::Components().decorations()[decorationId];

		decoration.positions.clear();
		decoration.colors.clear();

		for (auto& connectionParams : params)
		{
			if (connectionParams.segmentsNum > 1)
				connectionParams.segmentsNum = std::max((int)glm::distance(connectionParams.p1, connectionParams.p2) * 2, 2);

			const auto positions = connectionParams.getPositions();
			decoration.positions.insert(decoration.positions.end(), positions.begin(), positions.end());

			const auto colors = connectionParams.getColors();
			decoration.colors.insert(decoration.colors.end(), colors.begin(), colors.end());
		}

		assert(decoration.positions.size() == decoration.colors.size());

		decoration.state = ComponentState::Changed;
	}
}
