#pragma once

#include "_componentBase.hpp"
#include "_physical.hpp"

namespace Components
{
	struct Plane : ComponentBase, Physical
	{
		Plane() = default;

		Plane(Body body,
			TextureComponentVariant texture = std::monostate{},
			std::vector<glm::vec2> texCoord = {},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			Physical(std::move(body), TCM::Plane(getComponentId()), texture, renderingSetup, renderLayer, customShadersProgram)
		{
		}

		std::function<void()> step;

		bool connectIfApproaching = false;

		struct
		{
			glm::vec2 turningDelta{ 0.0f, 0.0f };
			bool autoRotation = false;
			float autoRotationFactor = 0.5f;
			bool throttling = false;
			bool magneticHook = false;
		} controls;

		struct
		{
			glm::vec2 previousCenter{ 0.0f, 0.0f };
			std::unique_ptr<b2Joint, b2JointDeleter> grappleJoint;
			ComponentId connectedGrappleId = 0;
			ComponentId weakConnectedGrappleId = 0;
		} details;
	};
}
