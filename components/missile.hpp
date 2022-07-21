#pragma once

#include "_componentBase.hpp"
#include "_renderable.hpp"

#include <tools/graphicsHelpers.hpp>
#include <tools/b2Helpers.hpp>

#include <commonTypes/bodyUserData.hpp>

namespace Components
{
	struct Missile : ComponentBase, Renderable
	{
		Missile() = default;

		Missile(Body body,
			TextureComponentVariant texture = std::monostate{},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			Renderable(texture, renderingSetup, renderLayer, customShadersProgram),
			body(std::move(body))
		{
			Tools::AccessUserData(*this->body).bodyComponentVariant = TCM::Missile(getComponentId());
		}

		Body body;
		
		std::function<void()> step;

		std::vector<glm::vec3> getVertices(bool transformed = false) const override
		{
			return transformed
				? Tools::Transform(Tools::GetVertices(*body), getModelMatrix())
				: Tools::GetVertices(*body);
		}

		glm::vec2 getCenter() const
		{
			return ToVec2<glm::vec2>(body->GetWorldCenter());
		}

		glm::mat4 getModelMatrix() const override
		{
			return Tools::GetModelMatrix(*body);
		}
	};
}
