#pragma once

#include "components/dynamicWall.hpp"

namespace Components
{
	struct Grapple : DynamicWall
	{
		Grapple() = default;

		Grapple(Body body,
			TextureComponentVariant texture = std::monostate{},
			std::optional<ComponentId>  renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			DynamicWall(std::move(body), texture, renderingSetup, renderLayer, customShadersProgram)
		{
		}

		float influenceRadius = 0.0f;

		struct
		{
			glm::vec2 previousCenter{ 0.0f, 0.0f };
		} details;

		void setComponentId(ComponentId id) override
		{
			ComponentBase::setComponentId(id);
			setBodyComponentVariant(TCM::Grapple(id, this));
		}
	};
}
