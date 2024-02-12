#pragma once

#include "_physical.hpp"

#include <tools/b2Helpers.hpp>

#include <ogl/buffers/genericBuffers.hpp>

#include <optional>

namespace Components
{
	struct StaticPolyline : Physical
	{
		StaticPolyline(const std::vector<glm::vec2>& vertices,
			Tools::BodyParams bodyParams = Tools::BodyParams{},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt) :
			Physical(Tools::CreatePolylineBody(vertices, bodyParams), std::monostate{}, renderingSetup, renderLayer, customShadersProgram)
		{
			drawMode = GL_LINE_STRIP;
			bufferDataUsage = GL_DYNAMIC_DRAW;
		}

		StaticPolyline(Tools::BodyParams bodyParams = Tools::BodyParams{},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt) :
			Physical(Tools::CreateEmptyBody(bodyParams), std::monostate{}, renderingSetup, renderLayer, customShadersProgram)
		{
			drawMode = GL_LINE_STRIP;
			bufferDataUsage = GL_DYNAMIC_DRAW;
		}

		std::function<std::vector<glm::vec3>(const glm::vec3&, const glm::vec3&)> segmentVerticesGenerator;
		std::function<void(std::vector<glm::vec3>&)> keyVerticesTransformer;

		void init(ComponentId id) override
		{
			ComponentBase::init(id);
			Tools::SetCollisionFilteringBits(*this->body, Globals::CollisionBits::polyline, Globals::CollisionBits::all);
			setBodyComponentVariant(TCM::StaticPolyline(id, this));
		}

		std::vector<glm::vec3> getVertices(bool transformed = false) const override
		{
			auto vertices = transformed
				? Tools::TransformMat4(Tools::GetVertices(*body), modelMatrixF())
				: Tools::GetVertices(*body);

			if ((!segmentVerticesGenerator && !keyVerticesTransformer) || vertices.empty())
				return vertices;
			
			std::vector<glm::vec3> customVertices;

			// Box2d keeps body's fixture in reverse order.
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
				loaded.buffers->setVerticesBuffer(getVertices());
		}

		void replaceFixtures(const std::vector<glm::vec2>& vertices, const Tools::BodyParams& bodyParams)
		{
			Tools::DestroyFixtures(body);
			Tools::CreatePolylineFixtures(body, vertices, bodyParams);
			Tools::SetCollisionFilteringBits(*body, Globals::CollisionBits::polyline, Globals::CollisionBits::all);
		}
	};

	struct DynamicPolyline : StaticPolyline
	{
		using StaticPolyline::StaticPolyline;

		void init(ComponentId id) override
		{
			ComponentBase::init(id);
			Tools::SetCollisionFilteringBits(*this->body, Globals::CollisionBits::polyline, Globals::CollisionBits::all);
			setBodyComponentVariant(TCM::DynamicPolyline(id, this));
		}
	};
}
