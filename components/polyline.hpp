#pragma once

#include "_physical.hpp"

#include <tools/b2Helpers.hpp>

#include <ogl/buffers/genericBuffers.hpp>

#include <optional>

namespace Components
{
	struct Polyline : Physical
	{
		Polyline(const std::vector<glm::vec2>& vertices,
			Tools::BodyParams bodyParams = Tools::BodyParams{}.sensor(true),
			RenderingSetupF renderingSetupF = nullptr,
			RenderLayer renderLayer = RenderLayer::Midground,
			ShadersUtils::AccessorBase* customShadersProgram = nullptr) :
			Physical(Tools::CreatePolylineBody(vertices, bodyParams), std::monostate{}, std::move(renderingSetupF), renderLayer, customShadersProgram)
		{
			drawMode = GL_LINE_STRIP;
			bufferDataUsage = GL_DYNAMIC_DRAW;
		}

		Polyline(Tools::BodyParams bodyParams = Tools::BodyParams{}.sensor(true),
			RenderingSetupF renderingSetupF = nullptr,
			RenderLayer renderLayer = RenderLayer::Midground,
			ShadersUtils::AccessorBase* customShadersProgram = nullptr) :
			Physical(Tools::CreateEmptyBody(bodyParams), std::monostate{}, std::move(renderingSetupF), renderLayer, customShadersProgram)
		{
			drawMode = GL_LINE_STRIP;
			bufferDataUsage = GL_DYNAMIC_DRAW;
		}

		std::function<std::vector<glm::vec3>(const glm::vec3&, const glm::vec3&)> segmentVerticesGenerator;
		std::function<void(std::vector<glm::vec3>&)> keyVerticesTransformer;

		void init(ComponentId id, bool static_) override
		{
			ComponentBase::init(id, static_);
			Tools::SetCollisionFilteringBits(*this->body, Globals::CollisionBits::polyline, Globals::CollisionBits::all);
			setBodyComponentVariant(CM::Polyline(*this));
		}

		std::vector<glm::vec3> getPositions(bool transformed = false) const override
		{
			auto vertices = transformed
				? Tools::TransformMat4(Tools::GetVertices(*body), modelMatrixF())
				: Tools::GetVertices(*body);

			if ((!segmentVerticesGenerator && !keyVerticesTransformer) || vertices.empty())
				return vertices;
			
			std::vector<glm::vec3> customVertices;

			// Box2d keeps body's fixture in reversed order.
			if (keyVerticesTransformer)
				keyVerticesTransformer(vertices);

			for (auto it = vertices.begin(); it != vertices.end() - 1; ++it)
			{
				auto segmentVertices = segmentVerticesGenerator(*it, *(it + 1));
				customVertices.insert(customVertices.end(), segmentVertices.begin(), segmentVertices.end());
			}

			return customVertices;
		}

		const std::vector<glm::vec2> getTexCoords(bool = false) const override
		{
			return texCoord;
		}

		void step() override
		{
			if (stepF)
				stepF();

			if (loaded.buffers)
				loaded.buffers->setPositionsBuffer(getPositions());
		}

		void replaceFixtures(const std::vector<glm::vec2>& vertices, const Tools::BodyParams& bodyParams = Tools::BodyParams{}.sensor(true))
		{
			Tools::DestroyFixtures(body);
			if (vertices.size() > 1)
				Tools::CreatePolylineFixtures(body, vertices, bodyParams);
			Tools::SetCollisionFilteringBits(*body, Globals::CollisionBits::polyline, Globals::CollisionBits::all);
		}
	};
}
