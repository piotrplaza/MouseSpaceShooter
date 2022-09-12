#pragma once

#include "_renderable.hpp"

#include <components/_typeComponentMappers.hpp>

#include <tools/b2Helpers.hpp>

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
			? Tools::Transform(Tools::GetVertices(*body), getModelMatrix())
			: Tools::GetVertices(*body);
	}

	virtual void setPosition(const glm::vec2& position)
	{
		body->SetTransform({ position.x, position.y }, body->GetAngle());
	}

	virtual void setRotation(float angle)
	{
		body->SetTransform(body->GetPosition(), angle);
	}

	virtual void resetKinematic()
	{
		body->SetLinearVelocity({ 0.0f, 0.0f });
		body->SetAngularVelocity(0.0f);
	}

	glm::vec2 getCenter() const override
	{
		return ToVec2<glm::vec2>(body->GetWorldCenter());
	}

	virtual float getAngle() const
	{
		return body->GetAngle();
	}

	virtual glm::vec2 getVelocity() const
	{
		return ToVec2<glm::vec2>(body->GetLinearVelocity());
	}

	virtual void enable(bool value) override
	{
		Renderable::enable(value);
		body->SetEnabled(value);
	}

	void setBodyComponentVariant(BodyComponentVariant bodyComponentVariant)
	{
		Tools::AccessUserData(*this->body).bodyComponentVariant = bodyComponentVariant;
	}
};
