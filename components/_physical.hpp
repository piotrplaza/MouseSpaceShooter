#pragma once

#include "_renderable.hpp"

#include <tools/b2Helpers.hpp>

#include <commonTypes/typeComponentMappers.hpp>
#include <commonTypes/bodyUserData.hpp>

#include <vector>

struct Physical : Renderable
{
	Physical() = default;

	Physical(Body body,
		TextureComponentVariant texture,
		std::optional<ComponentId> renderingSetup,
		RenderLayer renderLayer,
		std::optional<Shaders::ProgramId> customShadersProgram) :
		Renderable(texture, renderingSetup, renderLayer, customShadersProgram),
		body(std::move(body))
	{
	}

	Body body;

	glm::mat4 getModelMatrix() const override
	{
		return modelMatrixF
			? modelMatrixF()
			: Tools::GetModelMatrix(*this->body);
	}

	std::vector<glm::vec3> getVertices(bool transformed = false) const override
	{
		return transformed
			? Tools::TransformMat4(Tools::GetVertices(*body), getModelMatrix())
			: Tools::GetVertices(*body);
	}

	void setEnable(bool value) override
	{
		Renderable::setEnable(value);
		body->SetEnabled(value);
	}

	glm::vec3 getOrigin() const override
	{
		return { ToVec2<glm::vec2>(body->GetPosition()), 0.0f };
	}

	virtual glm::vec2 getMassCenter() const
	{
		return ToVec2<glm::vec2>(body->GetWorldCenter());
	}

	virtual void setOrigin(const glm::vec2& center)
	{
		body->SetTransform({ center.x, center.y }, body->GetAngle());
	}

	virtual void setAngle(float angle)
	{
		body->SetTransform(body->GetPosition(), angle);
	}

	virtual float getAngle() const
	{
		return body->GetAngle();
	}

	virtual void setVelocity(const glm::vec2& velocity)
	{
		body->SetLinearVelocity({ velocity.x, velocity.y });
	}

	virtual glm::vec2 getVelocity() const
	{
		return ToVec2<glm::vec2>(body->GetLinearVelocity());
	}

	virtual void resetKinematic()
	{
		body->SetLinearVelocity({ 0.0f, 0.0f });
		body->SetAngularVelocity(0.0f);
	}

	void setBodyComponentVariant(BodyComponentVariant bodyComponentVariant)
	{
		Tools::AccessUserData(*this->body).bodyComponentVariant = bodyComponentVariant;
	}

	void changeBody(Body body)
	{
		this->body = std::move(body);
		init(getComponentId());
		state = ComponentState::Changed;
	}
};
