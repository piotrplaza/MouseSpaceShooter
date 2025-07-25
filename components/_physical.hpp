#pragma once

#include "_renderable.hpp"

#include <tools/b2Helpers.hpp>

#include <commonTypes/componentMappers.hpp>
#include <commonTypes/bodyUserData.hpp>

#include <vector>

struct Physical : Renderable
{
	Physical() = default;

	Physical(Body body,
		AbstractTextureComponentVariant texture,
		RenderingSetupF renderingSetupF,
		RenderLayer renderLayer,
		ShadersUtils::AccessorBase* customShadersProgram) :
		Renderable(texture, std::move(renderingSetupF), renderLayer, customShadersProgram),
		body(std::move(body))
	{
		if (!this->body)
			return;

		modelMatrixF = [this]() { return Tools::GetModelMatrix(*this->body); };
		originF = [this]() { return glm::vec3(ToVec2<glm::vec2>(this->body->GetPosition()), 0.0f); };
	}

	Body body;

	std::vector<glm::vec3> getPositions(bool transformed = false) const override
	{
		return transformed
			? Tools::TransformMat4(Tools::GetVertices(*body), modelMatrixF())
			: Tools::GetVertices(*body);
	}

	void setEnabled(bool value) override
	{
		Renderable::setEnabled(value);
		body->SetEnabled(value);
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
		init(getComponentId(), isStatic());
		if (this->body)
		{
			modelMatrixF = [this]() { return Tools::GetModelMatrix(*this->body); };
			originF = [this]() { return glm::vec3(ToVec2<glm::vec2>(this->body->GetPosition()), 0.0f); };
		}
		state = ComponentState::Changed;
	}
};
