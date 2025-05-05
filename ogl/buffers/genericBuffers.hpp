#pragma once

#include <components/_renderable.hpp>

#include <globals/components.hpp>

#include <commonTypes/componentMappers.hpp>
#include <commonTypes/resolutionMode.hpp>

#include <ogl/shadersUtils.hpp>
#include <ogl/oglProxy.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

#include <optional>
#include <functional>
#include <vector>
#include <deque>
#include <variant>

namespace Buffers
{
	struct GenericSubBuffers
	{
		GenericSubBuffers(bool defaultVAO = false);
		GenericSubBuffers(const GenericSubBuffers&) = delete;
		GenericSubBuffers(GenericSubBuffers&& other) noexcept;

		~GenericSubBuffers();

		void setPositionsBuffer(const std::vector<glm::vec3>& positions);
		void setPositionsBuffer(glm::vec3 position, unsigned count);
		void allocateTFPositionsBuffer(unsigned count);
		void setColorsBuffer(const std::vector<glm::vec4>& colors);
		void setColorsBuffer(glm::vec4 color, unsigned count);
		void allocateTFColorsBuffer(unsigned count);
		void setVelocitiesAndTimesBuffer(const std::vector<glm::vec4>& velocitiesAndTimes);
		void setVelocitiesAndTimesBuffer(glm::vec4 velocityAndTime, unsigned count);
		void allocateTFVelocitiesAndTimesBuffer(unsigned count);
		void setHSizesAndAnglesBuffer(const std::vector<glm::vec3>& hSizesAndAngles);
		void setHSizesAndAnglesBuffer(glm::vec3 hSizeAndAngle, unsigned count);
		void allocateTFHSizesAndAnglesBuffer(unsigned count);
		void setTexCoordsBuffer(const std::vector<glm::vec2>& texCoords);
		void setNormalsBuffer(const std::vector<glm::vec3>& normals);
		void setInstancedTransformsBuffer(const std::vector<glm::mat4>& transforms);
		void setInstancedNormalTransformsBuffer(const std::vector<glm::mat3>& transforms);
		void setIndicesBuffer(const std::vector<unsigned>& indices);

		bool isInstancingActive() const;
		bool isIndicingActive() const;

		void swapActiveBuffers(GenericSubBuffers& other);

		static constexpr unsigned positionAttribIdx = 0;
		static constexpr unsigned colorAttribIdx = 1;
		static constexpr unsigned velocityAndTimeAttribIdx = 2;
		static constexpr unsigned hSizeAndAngleAttribIdx = 3;
		static constexpr unsigned texCoordAttribIdx = 4;
		static constexpr unsigned normalAttribIdx = 5;
		static constexpr unsigned instancedTransformAttribIdx = 6;
		static constexpr unsigned instancedNormalTransformAttribIdx = 10;

		RenderableDef* renderable = nullptr;

		GLuint vertexArray = 0;
		GLsizei drawCount = 0;
		GLsizei instanceCount = 0;

		GLuint positionsBuffer = 0;
		std::optional<GLuint> colorsBuffer;
		std::optional<GLuint> velocitiesAndTimesBuffer;
		std::optional<GLuint> hSizesAndAnglesBuffer;
		std::optional<GLuint> texCoordsBuffer;
		std::optional<GLuint> normalsBuffer;
		std::optional<GLuint> instancedTransformsBuffer;
		std::optional<GLuint> instancedNormalTransformsBuffer;
		std::optional<GLuint> indicesBuffer;

	private:
		void createPositionsBuffer(unsigned target = GL_ARRAY_BUFFER);
		void createColorsBuffer(unsigned target = GL_ARRAY_BUFFER);
		void createVelocitiesAndTimesBuffer(unsigned target = GL_ARRAY_BUFFER);
		void createHSizesAndAnglesBuffer(unsigned target = GL_ARRAY_BUFFER);
		void createTexCoordsBuffer(unsigned target = GL_ARRAY_BUFFER);
		void createNormalsBuffer(unsigned target = GL_ARRAY_BUFFER);
		void createInstancedTransformsBuffer(unsigned target = GL_ARRAY_BUFFER);
		void createInstancedNormalTransformsBuffer(unsigned target = GL_ARRAY_BUFFER);
		void createIndicesBuffer();

		size_t numOfAllocatedPositions = 0;
		size_t numOfAllocatedColors = 0;
		size_t numOfAllocatedVelocitiesAndTimes = 0;
		size_t numOfAllocatedHSizesAndAngles = 0;
		size_t numOfAllocatedTexCoords = 0;
		size_t numOfAllocatedNormals = 0;
		size_t numOfAllocatedInstancedTransforms = 0;
		size_t numOfAllocatedInstancedNormalTransforms = 0;
		size_t numOfAllocatedIndices = 0;
		std::optional<GLenum> allocatedBufferDataUsage;

		bool expired = false;
	};

	struct GenericBuffers : GenericSubBuffers
	{
		GenericBuffers(bool defaultVAO = false);
		GenericBuffers(const GenericBuffers&) = delete;
		GenericBuffers(GenericBuffers&& other) noexcept;

		const std::vector<glm::mat3>& calcNormalTransforms(const std::vector<glm::mat4>& transforms);

		void bindActiveTFBuffers() const;

		void applyComponent(auto& renderableComponent, bool staticComponent)
		{
			auto applyMainPart = [&]() {
				GenericSubBuffers::renderable = &renderableComponent;
				renderable = &renderableComponent;

				if (!staticComponent && (!renderableComponent.isEnabled() || !renderableComponent.renderF()) && renderableComponent.loaded.buffers)
					return;

				RenderableComponentCommonsToBuffersCommons(renderableComponent, *this);

				if (renderableComponent.instancing)
				{
					setInstancedTransformsBuffer(renderableComponent.instancing->transforms_);
					if (renderableComponent.params3D && !renderableComponent.params3D->gpuSideInstancedNormalTransforms_)
						setInstancedNormalTransformsBuffer(calcNormalTransforms(renderableComponent.instancing->transforms_));
				}
				};

			auto applySubsequencePart = [&]() {
				auto subBuffersIt = subsequence.begin();
				for (auto& renderableDef : renderableComponent.subsequence)
				{
					auto& subBuffers = ReuseOrEmplaceBack(subsequence, subBuffersIt);

					subBuffers.renderable = &renderableDef;
					renderableDef.loaded.subBuffers = &subBuffers;

					if (!staticComponent && (!renderableComponent.isEnabled() || !renderableComponent.renderF()) && renderableComponent.loaded.buffers)
						continue;

					RenderableComponentCommonsToBuffersCommons(renderableDef, subBuffers);
				}
				subsequence.resize(std::distance(subsequence.begin(), subBuffersIt));
				};

			applyMainPart();
			applySubsequencePart();

			renderableComponent.loaded.buffers = this;
		}

		void applyTFComponent(auto& renderableComponent, bool staticComponent)
		{
			auto applyMainPart = [&]() {
				GenericSubBuffers::renderable = &renderableComponent;
				renderable = &renderableComponent;

				if (!staticComponent && (!renderableComponent.isEnabled() || !renderableComponent.renderF()) && renderableComponent.loaded.tfBuffers)
					return;

				TFComponentCommonsToBuffersCommons(renderableComponent, *this);
			};

			auto applySubsequencePart = [&]() {
				auto subBuffersIt = subsequence.begin();
				for (auto& renderableDef : renderableComponent.subsequence)
				{
					auto& subBuffers = ReuseOrEmplaceBack(subsequence, subBuffersIt);

					subBuffers.renderable = &renderableDef;
					renderableDef.loaded.subBuffers = &subBuffers;

					if (!staticComponent && (!renderableComponent.isEnabled() || !renderableComponent.renderF()) && renderableComponent.loaded.tfBuffers)
						continue;

					TFComponentCommonsToBuffersCommons(renderableDef, subBuffers);
				}
				subsequence.resize(std::distance(subsequence.begin(), subBuffersIt));
			};

			applyMainPart();
			applySubsequencePart();

			renderableComponent.loaded.tfBuffers = this;
		}

		template <typename GeneralSetup, typename PostSetup, typename GeneralTeardown, typename PostTeardown>
		void draw(ShadersUtils::AccessorBase& program, GeneralSetup generalSetup, PostSetup postSetup, GeneralTeardown generalTeardown, PostTeardown postTeardown, bool transformFeedback = false) const
		{
			if (!renderable || renderable->state == ComponentState::Outdated || !renderable->isEnabled())
				return;

			auto setAndDraw = [&](const GenericSubBuffers& buffers)
			{
				if (!(buffers.renderable->renderF)())
					return;

				glProxyBindVertexArray(buffers.vertexArray);

				generalSetup(buffers);

				std::function<void()> renderingTeardown;

				if (transformFeedback)
				{
					if (buffers.renderable->tfRenderingSetupF)
						renderingTeardown = buffers.renderable->tfRenderingSetupF(program);
				}
				else if (buffers.renderable->renderingSetupF)
					renderingTeardown = buffers.renderable->renderingSetupF(program);

				postSetup(buffers);

				if (isInstancingActive())
				{
					if (isIndicingActive())
						glDrawElementsInstanced(buffers.renderable->drawMode, buffers.drawCount, GL_UNSIGNED_INT, nullptr, instanceCount);
					else
						glDrawArraysInstanced(buffers.renderable->drawMode, 0, buffers.drawCount, instanceCount);
				}
				else
				{
					if (isIndicingActive())
						glDrawElements(buffers.renderable->drawMode, buffers.drawCount, GL_UNSIGNED_INT, nullptr);
					else
						glDrawArrays(buffers.renderable->drawMode, 0, buffers.drawCount);
				}

				generalTeardown(buffers);

				if (renderingTeardown)
					renderingTeardown();

				postTeardown(buffers);
			};

			for (unsigned i = 0; i < subsequence.size(); ++i)
			{
				const unsigned id = (i + renderable->subsequenceBegin) % subsequence.size();
				if (id == renderable->posInSubsequence)
					setAndDraw(*this);
				setAndDraw(subsequence[id]);
			}

			if (subsequence.empty() || renderable->posInSubsequence == subsequence.size())
				setAndDraw(*this);
		}

		Renderable* renderable = nullptr;

		std::deque<GenericSubBuffers> subsequence;
		std::vector<glm::mat3> normalTransforms;

	private:
		inline void RenderableComponentCommonsToBuffersCommons(auto& renderableDef, Buffers::GenericSubBuffers& buffers)
		{
			if (renderableDef.forcedPositionsCount)
			{
				assert(renderableDef.positions.size() == 1);
				buffers.setPositionsBuffer(renderableDef.getPositions()[0], renderableDef.forcedPositionsCount);
			}
			else
				buffers.setPositionsBuffer(renderableDef.getPositions());

			if (!renderableDef.colors.empty())
				if (renderableDef.forcedColorsCount)
				{
					assert(renderableDef.colors.size() == 1);
					buffers.setColorsBuffer(renderableDef.colors[0], renderableDef.forcedColorsCount);
				}
				else
					buffers.setColorsBuffer(renderableDef.colors);

			if constexpr (requires { renderableDef.velocitiesAndTimes; })
				if (!renderableDef.velocitiesAndTimes.empty())
					if (renderableDef.forcedVelocitiesAndTimesCount)
					{
						assert(renderableDef.velocitiesAndTimes.size() == 1);
						buffers.setVelocitiesAndTimesBuffer(renderableDef.velocitiesAndTimes[0], renderableDef.forcedVelocitiesAndTimesCount);
					}
					else
						buffers.setVelocitiesAndTimesBuffer(renderableDef.velocitiesAndTimes);

			if constexpr (requires { renderableDef.hSizesAndAngles; })
				if (!renderableDef.hSizesAndAngles.empty())
					if (renderableDef.forcedHSizesAndAnglesCount)
					{
						assert(renderableDef.hSizesAndAngles.size() == 1);
						buffers.setHSizesAndAnglesBuffer(renderableDef.hSizesAndAngles[0], renderableDef.forcedHSizesAndAnglesCount);
					}
					else
						buffers.setHSizesAndAnglesBuffer(renderableDef.hSizesAndAngles);

			if (!std::holds_alternative<std::monostate>(renderableDef.texture))
				buffers.setTexCoordsBuffer(renderableDef.getTexCoords());

			if (renderableDef.params3D)
				buffers.setNormalsBuffer(renderableDef.params3D->normals_);

			buffers.setIndicesBuffer(renderableDef.getIndices());
		}

		inline void TFComponentCommonsToBuffersCommons(auto& renderableDef, Buffers::GenericSubBuffers& buffers)
		{
			buffers.allocateTFPositionsBuffer(renderableDef.forcedPositionsCount
				? renderableDef.forcedPositionsCount
				: (unsigned)renderableDef.positions.size());

			if (!renderableDef.colors.empty())
				buffers.allocateTFColorsBuffer(renderableDef.forcedColorsCount
					? renderableDef.forcedColorsCount
					: (unsigned)renderableDef.colors.size());

			if constexpr (requires { renderableDef.velocitiesAndTimes; })
				if (!renderableDef.velocitiesAndTimes.empty())
					buffers.allocateTFVelocitiesAndTimesBuffer(renderableDef.forcedVelocitiesAndTimesCount
						? renderableDef.forcedVelocitiesAndTimesCount
						: (unsigned)renderableDef.velocitiesAndTimes.size());

			if constexpr (requires { renderableDef.hSizesAndAngles; })
				if (!renderableDef.hSizesAndAngles.empty())
					buffers.allocateTFHSizesAndAnglesBuffer(renderableDef.forcedHSizesAndAnglesCount
						? renderableDef.forcedHSizesAndAnglesCount
						: (unsigned)renderableDef.hSizesAndAngles.size());
		}

		template <typename BuffersContainer>
		inline typename BuffersContainer::value_type& ReuseOrEmplaceBack(BuffersContainer& buffersContainer, typename BuffersContainer::iterator& it)
		{
			return it == buffersContainer.end()
				? buffersContainer.emplace_back(), it = buffersContainer.end(), buffersContainer.back()
				: *it++;
		}
	};
}
