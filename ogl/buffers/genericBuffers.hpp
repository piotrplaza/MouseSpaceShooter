#pragma once

#include <ogl/shaders.hpp>

#include <commonTypes/resolutionMode.hpp>
#include <components/_typeComponentMappers.hpp>
#include <components/_typeComponentMappers.hpp>
#include <components/renderingSetup.hpp>

#include <globals/components.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <optional>
#include <functional>
#include <vector>
#include <variant>

namespace Buffers
{
	struct GenericSubBuffers
	{
		GenericSubBuffers();
		GenericSubBuffers(const GenericSubBuffers&) = delete;
		GenericSubBuffers(GenericSubBuffers&& other) noexcept;

		~GenericSubBuffers();

		void allocateOrUpdateVerticesBuffer(const std::vector<glm::vec3>& vertices);
		void allocateOrUpdateColorsBuffer(const std::vector<glm::vec4>& colors);
		void allocateOrUpdateTexCoordBuffer(const std::vector<glm::vec2>& texCoord);

		std::function<glm::mat4()> modelMatrixF;
		std::function<glm::vec4()> *colorF = nullptr;
		TextureComponentVariant* texture = nullptr;
		std::optional<ComponentId>* renderingSetup = nullptr;
		std::optional<Shaders::ProgramId>* customShadersProgram = nullptr;
		GLenum* drawMode = nullptr;
		GLenum* bufferDataUsage = nullptr;
		bool* preserveTextureRatio = nullptr;
		bool* render = nullptr;

		GLuint vertexArray;
		size_t numOfVertices;

	private:
		void createColorBuffer();
		void createTexCoordBuffer();

		GLuint positionBuffer;
		std::optional<GLuint> colorBuffer;
		std::optional<GLuint> texCoordBuffer;
		size_t numOfAllocatedVertices = 0;
		size_t numOfAllocatedColors = 0;
		size_t numOfAllocatedTexCoord = 0;
		std::optional<GLenum> allocatedBufferDataUsage;

		bool expired = false;
	};

	struct GenericBuffers : GenericSubBuffers
	{
		template <typename GeneralSetup>
		void draw(Shaders::ProgramId programId, GeneralSetup generalSetup) const
		{
			auto setAndDraw = [&](const GenericSubBuffers& buffers)
			{
				if (!*buffers.render)
					return;

				glBindVertexArray(buffers.vertexArray);

				generalSetup(buffers);

				std::function<void()> renderingTeardown;
				if (*buffers.renderingSetup)
					renderingTeardown = Globals::Components().renderingSetups()[**buffers.renderingSetup](programId);

				glDrawArrays(*buffers.drawMode, 0, buffers.numOfVertices);

				if (renderingTeardown)
					renderingTeardown();
			};

			for (unsigned i = 0; i < subsequence.size(); ++i)
			{
				const unsigned id = (i + *subsequenceBegin) % subsequence.size();
				if (id == *posInSubsequence)
					setAndDraw(*this);
				setAndDraw(subsequence[id]);
			}

			if (subsequence.empty() || *posInSubsequence == subsequence.size())
				setAndDraw(*this);
		}

		template <typename Component>
		void applyComponentSubsequence(Component& component)
		{
			subsequence.reserve(component.subsequence.size());
			auto subBuffersIt = subsequence.begin();
			for (auto& subComponent : component.subsequence)
			{
				auto& subBuffers = reuseOrEmplaceBack(subsequence, subBuffersIt);
				componentCommonsToBuffersCommons(subComponent, subBuffers);
				subComponent.loaded.subBuffers = &subBuffers;
			}
			subsequence.resize(std::distance(subsequence.begin(), subBuffersIt));
		}

		template <typename Component>
		void applyComponent(Component& component)
		{
			customShadersProgram = &component.customShadersProgram;
			resolutionMode = &component.resolutionMode;
			subsequenceBegin = &component.subsequenceBegin;
			posInSubsequence = &component.posInSubsequence;

			componentCommonsToBuffersCommons(component, *this);

			sourceComponent = component.getComponentId();
			component.loaded.buffers = this;
			component.state = ComponentState::Ongoing;
		}

		ResolutionMode* resolutionMode = nullptr;

		std::vector<GenericSubBuffers> subsequence;
		unsigned* subsequenceBegin = nullptr;
		unsigned* posInSubsequence = nullptr;

		ComponentId sourceComponent = 0;

	private:
		template <typename Buffers>
		Buffers& reuseOrEmplaceBack(std::vector<Buffers>& buffers, typename std::vector<Buffers>::iterator& it)
		{
			return it == buffers.end()
				? buffers.emplace_back(), it = buffers.end(), buffers.back()
				: *it++;
		}

		template <typename SubComponent>
		void componentCommonsToBuffersCommons(SubComponent& subComponent, Buffers::GenericSubBuffers& buffers)
		{
			buffers.modelMatrixF = [&]() { return subComponent.getModelMatrix(); };
			buffers.colorF = &subComponent.colorF;
			buffers.renderingSetup = &subComponent.renderingSetup;
			buffers.texture = &subComponent.texture;
			buffers.drawMode = &subComponent.drawMode;
			buffers.bufferDataUsage = &subComponent.bufferDataUsage;
			buffers.preserveTextureRatio = &subComponent.preserveTextureRatio;
			buffers.render = &subComponent.render;

			buffers.allocateOrUpdateVerticesBuffer(subComponent.getVertices());
			buffers.allocateOrUpdateColorsBuffer(subComponent.getColors());
			buffers.allocateOrUpdateTexCoordBuffer(subComponent.getTexCoord());
		}
	};
}
