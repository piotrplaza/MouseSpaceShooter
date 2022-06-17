#include "actors.hpp"

#include <ogl/oglProxy.hpp>
#include <ogl/buffersHelpers.hpp>
#include <ogl/renderingHelpers.hpp>

#include <ogl/shaders/basic.hpp>
#include <ogl/shaders/textured.hpp>

#include <tools/b2Helpers.hpp>
#include <tools/graphicsHelpers.hpp>

#include <components/plane.hpp>
#include <components/physics.hpp>
#include <components/mvp.hpp>
#include <components/renderingSetup.hpp>
#include <components/grapple.hpp>
#include <components/texture.hpp>
#include <components/graphicsSettings.hpp>

#include <globals/shaders.hpp>

#include <Box2D/Box2D.h>

#include <GL/glew.h>

#include <vector>
#include <limits>
#include <algorithm>

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
		initGraphics();
	}

	void Actors::step()
	{
		Globals::ForEach(Globals::Components().planes(), [&](auto& plane) {
			turn(plane);
			throttle(plane);
			magneticHook(plane);

			connections.updateBuffers();
			});
	}

	void Actors::render() const
	{
		basicRender();
		coloredRender();
		texturedRender();
		customShadersRender();
	}

	void Actors::initGraphics()
	{
		updatePlanesStaticBuffers();
	}

	void Actors::updatePlanesStaticBuffers()
	{
		Tools::UpdateStaticBuffers(Globals::Components().planes(), simplePlaneBuffers, texturedPlaneBuffers, customShadersPlaneBuffers);
	}

	void Actors::basicRender() const
	{
		glUseProgram_proxy(Globals::Shaders().basic().getProgramId());
		Globals::Shaders().basic().vp(Globals::Components().mvp().getVP());
		Globals::Shaders().basic().color(Globals::Components().graphicsSettings().defaultColor);

		for (const auto& simplePlaneBuffers : simplePlaneBuffers)
		{
			Globals::Shaders().basic().color(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().basic().model(simplePlaneBuffers.modelMatrixF ? simplePlaneBuffers.modelMatrixF() : glm::mat4(1.0f));

			std::function<void()> renderingTeardown =
				Globals::Components().renderingSetups()[simplePlaneBuffers.renderingSetup](Globals::Shaders().basic().getProgramId());

			simplePlaneBuffers.draw();

			if (renderingTeardown)
				renderingTeardown();
		}
	}

	void Actors::texturedRender() const
	{
		glUseProgram_proxy(Globals::Shaders().textured().getProgramId());
		Globals::Shaders().textured().vp(Globals::Components().mvp().getVP());

		for (const auto& customTexturedPlaneBuffers : texturedPlaneBuffers)
		{
			Globals::Shaders().textured().color(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().textured().model(customTexturedPlaneBuffers.modelMatrixF ? customTexturedPlaneBuffers.modelMatrixF() : glm::mat4(1.0f));
			Tools::TexturedRender(Globals::Shaders().textured(), customTexturedPlaneBuffers,
				customTexturedPlaneBuffers.texture);
		}
	}

	void Actors::customShadersRender() const
	{
		for (const auto& currentBuffers : customShadersPlaneBuffers)
		{
			glUseProgram_proxy(*currentBuffers.customShadersProgram);

			std::function<void()> renderingTeardown;
			if (currentBuffers.renderingSetup)
				renderingTeardown = Globals::Components().renderingSetups()[currentBuffers.renderingSetup](*currentBuffers.customShadersProgram);

			currentBuffers.draw();

			if (renderingTeardown)
				renderingTeardown();
		}
	}

	void Actors::coloredRender() const
	{
		glUseProgram_proxy(Globals::Shaders().basic().getProgramId());
		Globals::Shaders().basic().vp(Globals::Components().mvp().getVP());
		Globals::Shaders().basic().color(Globals::Components().graphicsSettings().defaultColor);
		Globals::Shaders().basic().model(glm::mat4(1.0f));
		connections.buffers.draw();
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

		if (plane.grappleJoint && plane.controls.autoRotation)
		{
			const auto& grapple = Globals::Components().grapples()[plane.connectedGrappleId];
			const glm::vec2 stepVelocity = (plane.getCenter() - plane.previousCenter) - (grapple.getCenter() - grapple.previousCenter);
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

		for (ComponentId i = 1; i < (ComponentId)Globals::Components().grapples().size(); ++i)
		{
			const auto& grapple = Globals::Components().grapples()[i];
			const float grappleDistance = glm::distance(plane.getCenter(), grapple.getCenter());

			if (grappleDistance > grapple.influenceRadius) continue;

			grapplesInRange.push_back(i);

			if (grappleDistance < nearestGrappleDistance)
			{
				nearestGrappleDistance = grappleDistance;
				nearestGrappleId = i;
			}
		}

		if (!plane.controls.magneticHook)
		{
			plane.connectedGrappleId = 0;
			plane.weakConnectedGrappleId = 0;
			plane.grappleJoint.reset();
		}

		for (ComponentId grappleInRange : grapplesInRange)
		{
			const auto& grapple = Globals::Components().grapples()[grappleInRange];

			if (grappleInRange == nearestGrappleId)
			{
				if (plane.controls.magneticHook && !plane.grappleJoint &&
					(glm::distance(plane.getCenter(), grapple.getCenter()) >=
					glm::distance(plane.previousCenter, grapple.previousCenter) || 
					plane.connectIfApproaching))
				{
					plane.connectedGrappleId = grappleInRange;
					plane.weakConnectedGrappleId = 0;
					createGrappleJoint(plane);
				}
				else if(plane.connectedGrappleId != grappleInRange)
				{
					if (plane.controls.magneticHook)
					{
						if (plane.grappleJoint)
						{
							connections.params.emplace_back(plane.getCenter(), grapple.getCenter(),
								glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) * 0.2f, 1);
						}
						plane.weakConnectedGrappleId = grappleInRange;
					}
					else
					{
						connections.params.emplace_back(plane.getCenter(), grapple.getCenter(),
							glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) * 0.2f, 1);
					}
				}
			}
			else if (plane.connectedGrappleId != grappleInRange)
			{
				connections.params.emplace_back(plane.getCenter(), grapple.getCenter(), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) * 0.2f, 1);
			}
		}

		if (plane.connectedGrappleId)
		{
			connections.params.emplace_back(plane.getCenter(), Globals::Components().grapples()[plane.connectedGrappleId].getCenter(),
				glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) * 0.7f, 20, 0.4f);
		}
		else if (plane.weakConnectedGrappleId)
		{
			connections.params.emplace_back(plane.getCenter(), Globals::Components().grapples()[plane.weakConnectedGrappleId].getCenter(),
				glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) * 0.5f, 1);
		}
	}

	void Actors::createGrappleJoint(Components::Plane& plane) const
	{
		assert(plane.connectedGrappleId);

		const auto& grapple = Globals::Components().grapples()[plane.connectedGrappleId];

		b2DistanceJointDef distanceJointDef;
		distanceJointDef.bodyA = plane.body.get();
		distanceJointDef.bodyB = grapple.body.get();
		distanceJointDef.localAnchorA = distanceJointDef.bodyA->GetLocalCenter();
		distanceJointDef.localAnchorB = distanceJointDef.bodyB->GetLocalCenter();
		distanceJointDef.length = glm::distance(plane.getCenter(), grapple.getCenter());
		distanceJointDef.collideConnected = true;
		plane.grappleJoint.reset(Globals::Components().physics().world->CreateJoint(&distanceJointDef));
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

	void Actors::Connections::updateBuffers()
	{
		vertices.clear();
		colors.clear();

		for (auto& connectionParams : params)
		{
			if (connectionParams.segmentsNum > 1)
				connectionParams.segmentsNum = std::max((int)glm::distance(connectionParams.p1, connectionParams.p2) * 2, 2);

			const auto vertices = connectionParams.getVertices();
			this->vertices.insert(this->vertices.end(), vertices.begin(), vertices.end());

			const auto colors = connectionParams.getColors();
			this->colors.insert(this->colors.end(), colors.begin(), colors.end());
		}

		buffers.allocateOrUpdatePositionsBuffer(vertices);
		buffers.allocateOrUpdateColorsBuffer(colors);

		assert(vertices.size() == colors.size());
	}
}
