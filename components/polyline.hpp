#pragma once

#include "_physical.hpp"

#include <tools/b2Helpers.hpp>

#include <ogl/buffers/genericBuffers.hpp>

#include <optional>

namespace Components
{
	struct Polyline : Physical
	{
		Polyline() = default;

		Polyline(const std::vector<glm::vec2>& vertices,
			Tools::BodyParams bodyParams = Tools::BodyParams{},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt) :
			Physical(Tools::CreatePolylineBody(vertices, bodyParams), std::monostate{}, renderingSetup, renderLayer, customShadersProgram)
		{
			Tools::SetCollisionFilteringBits(*this->body, Globals::CollisionBits::polyline, Globals::CollisionBits::all);
			drawMode = GL_LINES;
			bufferDataUsage = GL_DYNAMIC_DRAW;
		}

		std::function<std::vector<glm::vec3>(const glm::vec3&, const glm::vec3&)> segmentVerticesGenerator;
		std::function<glm::vec3(const glm::vec3&)> keyVerticesTransformer;
		bool loop = false;

		void setComponentId(ComponentId id) override
		{
			ComponentBase::setComponentId(id);
			setBodyComponentVariant(TCM::Polyline(id, this));
		}

		std::vector<glm::vec3> getVertices(bool transformed = false) const override
		{
			const auto vertices = transformed
				? Tools::Transform(Tools::GetVertices(*body), getModelMatrix())
				: Tools::GetVertices(*body);

			assert(vertices.size() >= 2 && vertices.size() % 2 == 0);

			if (!segmentVerticesGenerator && !keyVerticesTransformer)
				return vertices;
			
			std::vector<glm::vec3> customVertices;

			// Box2d keeps body's fixture in reverse order.
			glm::vec3 nextSegmentBegin;
			glm::vec3 lastSegmentEnd;
			for (auto it = vertices.begin(); it != vertices.end(); it += 2)
			{
				std::pair<glm::vec3, glm::vec3> segmentV = [&]() {
					if (!keyVerticesTransformer)
						return std::make_pair(*it, *(it + 1));
					else
					{
						const glm::vec3 v1 = loop && it == std::prev(vertices.end(), 2)
							? lastSegmentEnd
							: keyVerticesTransformer(*it);
						const glm::vec3 v2 = it == vertices.begin()
							? lastSegmentEnd = keyVerticesTransformer(*(it + 1))
							: nextSegmentBegin;
						nextSegmentBegin = v1;

						return std::make_pair(v1, v2);
					}
				}();

				auto segmentVertices = segmentVerticesGenerator(segmentV.first, segmentV.second);
				customVertices.insert(customVertices.end(), segmentVertices.begin(), segmentVertices.end());
			}

			return customVertices;
		}

		const std::vector<glm::vec2> getTexCoord(bool = false) const override
		{
			return texCoord;
		}

		void step() override
		{
			loaded.buffers->setVerticesBuffer(getVertices());
		}
	};
}
