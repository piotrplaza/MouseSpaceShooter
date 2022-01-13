#pragma once

#include <components/texture.hpp>
#include <components/blendingTexture.hpp>
#include <components/animatedTexture.hpp>
#include <components/mvp.hpp>
#include <components/screenInfo.hpp>
#include <components/physics.hpp>
#include <components/mouseState.hpp>

#include <components/typeComponentMappers.hpp>

#include <ogl/oglProxy.hpp>

#include <tools/utility.hpp>

#include <GL/glew.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <functional>
#include <optional>
#include <array>
#include <variant>

namespace Tools
{
	template <typename ShadersProgram>
	inline void MVPInitialization(ShadersProgram& shadersProgram, std::optional<glm::mat4> modelMatrix = std::nullopt)
	{
		shadersProgram.vpUniform(Globals::Components().mvp().getVP());
		shadersProgram.modelUniform(modelMatrix ? *modelMatrix : glm::mat4(1.0f));
	}

	template <typename ShadersProgram>
	inline void StaticTexturedRenderInitialization(ShadersProgram& shadersProgram, unsigned textureId, bool textureRatioPreserved)
	{
		const auto& textureComponent = Globals::Components().textures()[textureId];

		shadersProgram.numOfTexturesUniform(1);
		shadersProgram.texturesUniform(0, textureId);
		shadersProgram.texturesTranslateUniform(0, textureComponent.translate);
		shadersProgram.texturesScaleUniform(0,
			{ (textureRatioPreserved ? (float)textureComponent.loaded.size.x / textureComponent.loaded.size.y : 1.0f)
			* textureComponent.scale.x, textureComponent.scale.y });
	}

	template <typename ShadersProgram>
	inline void AnimatedTexturedRenderInitialization(ShadersProgram& shadersProgram, unsigned animatedTextureId)
	{
		const auto& animationTextureComponent = Globals::Components().animatedTextures()[animatedTextureId];

		shadersProgram.numOfTexturesUniform(1);
		shadersProgram.texturesUniform(0, animationTextureComponent.getTextureId());
		const auto frameTransformation = animationTextureComponent.getFrameTransformation();
		shadersProgram.texturesTranslateUniform(0, frameTransformation.translate);
		shadersProgram.texturesScaleUniform(0, frameTransformation.scale);
	}

	template <typename ShadersProgram>
	inline void BlendingTexturedRenderInitialization(ShadersProgram& shadersProgram, unsigned blendingTextureId, bool textureRatioPreserved)
	{
		const auto& blendingTextureComponent = Globals::Components().blendingTextures()[blendingTextureId];

		unsigned iStart = 0;
		if (blendingTextureComponent.blendingAnimation)
		{
			iStart = 1;
			AnimatedTexturedRenderInitialization(shadersProgram, blendingTextureComponent.texturesIds[0]);
		}

		shadersProgram.numOfTexturesUniform(blendingTextureComponent.texturesIds.size());
		for (unsigned i = iStart; i < (unsigned)blendingTextureComponent.texturesIds.size(); ++i)
		{
			const auto textureId = blendingTextureComponent.texturesIds[i];
			const auto& textureComponent = Globals::Components().textures()[textureId];

			shadersProgram.texturesUniform(i, textureId);
			shadersProgram.texturesTranslateUniform(i, textureComponent.translate);
			shadersProgram.texturesScaleUniform(i,
				{ (textureRatioPreserved ? (float)textureComponent.loaded.size.x / textureComponent.loaded.size.y : 1.0f)
				* textureComponent.scale.x, textureComponent.scale.y });
		}
	}

	template <typename ShadersProgram>
	class TexturedRenderInitializationVisitor
	{
	public:
		TexturedRenderInitializationVisitor(ShadersProgram& shadersProgram, bool textureRatioPreserved):
			shadersProgram(shadersProgram),
			textureRatioPreserved(textureRatioPreserved)
		{
		}

		void operator ()(TCM::Texture texture)
		{
			StaticTexturedRenderInitialization(shadersProgram, texture.id, textureRatioPreserved);
		}

		void operator ()(TCM::AnimatedTexture animatedTexture)
		{
			AnimatedTexturedRenderInitialization(shadersProgram, animatedTexture.id);
		}

		void operator ()(TCM::BlendingTexture blendingTexture)
		{
			BlendingTexturedRenderInitialization(shadersProgram, blendingTexture.id, textureRatioPreserved);
		}

		void operator ()(std::monostate)
		{
			assert(!"Wrong variant state.");
		}

	private:
		ShadersProgram& shadersProgram;
		bool textureRatioPreserved;
	};

	template <typename ShadersProgram, typename Buffers>
	inline void TexturedRender(ShadersProgram& shadersProgram, const Buffers& buffers, const TextureVariant& texture)
	{
		std::visit(TexturedRenderInitializationVisitor{ shadersProgram, buffers.textureRatioPreserved }, texture);

		std::function<void()> renderingTeardown;
		if (buffers.renderingSetup)
			renderingTeardown = Globals::Components().renderingSetups()[buffers.renderingSetup](shadersProgram.getProgramId());

		glBindVertexArray(buffers.vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, buffers.positionsCache.size());
		
		if (renderingTeardown)
			renderingTeardown();
	}

	template <typename ShadersPrograms>
	inline void TexturedScreenRender(ShadersPrograms& shadersProgram, unsigned texture, std::function<void()> customSetup = nullptr,
		std::function<std::array<glm::vec3, 6>()> positionsGenerator = nullptr)
	{
		const int numOfVertices = 6;

		const static glm::vec3 defaultPositions[numOfVertices] =
		{
			{-1.0f, -1.0f, 0.0f},
			{1.0f, -1.0f, 0.0f},
			{-1.0f, 1.0f, 0.0f},
			{-1.0f, 1.0f, 0.0f},
			{1.0f, -1.0f, 0.0f},
			{1.0f, 1.0f, 0.0f}
		};

		const static glm::vec2 defaultTexCoords[numOfVertices] =
		{
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{0.0f, 1.0f},
			{0.0f, 1.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f}
		};

		static std::array<glm::vec3, numOfVertices> customPositions;

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		if (positionsGenerator)
		{
			customPositions = positionsGenerator();
			glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, &customPositions);
		}
		else
			glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, &defaultPositions);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, &defaultTexCoords);
		glEnableVertexAttribArray(1);

		glUseProgram_proxy(shadersProgram.getProgramId());

		shadersProgram.modelUniform(glm::mat4(1.0f));
		shadersProgram.vpUniform(glm::mat4(1.0f));
		shadersProgram.colorUniform(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		shadersProgram.numOfTexturesUniform(1);
		shadersProgram.texturesUniform(0, texture);
		shadersProgram.texturesTranslateUniform(0, glm::vec2(0.0f));
		shadersProgram.texturesScaleUniform(0, glm::vec2(1.0f));

		if (customSetup)
			customSetup();

		glDrawArrays(GL_TRIANGLES, 0, numOfVertices);
	}

	template <typename ShadersPrograms>
	auto StandardFullscreenRenderer(ShadersPrograms& shadersProgram)
	{
		const auto& screenInfo = Globals::Components().screenInfo();

		return[&](unsigned textureId)
		{
			glDisable(GL_BLEND);
			TexturedScreenRender(shadersProgram, textureId, nullptr, [&]()
				{
					const float quakeIntensity = 0.001f * Globals::Components().shockwaves().size();
					const glm::vec2 quakeIntensityXY = screenInfo.windowSize.x > screenInfo.windowSize.y
						? glm::vec2(quakeIntensity, quakeIntensity * (float)screenInfo.windowSize.x / screenInfo.windowSize.y)
						: glm::vec2(quakeIntensity * (float)screenInfo.windowSize.y / screenInfo.windowSize.x, quakeIntensity);

					const glm::vec3 p1 = { -1.0f - Tools::Random(0.0f, quakeIntensityXY.x), -1.0f - Tools::Random(0.0f, quakeIntensityXY.y), 0.0f };
					const glm::vec3 p2 = { 1.0f + Tools::Random(0.0f, quakeIntensityXY.x), p1.y, 0.0f };
					const glm::vec3 p3 = { p1.x, 1.0f + Tools::Random(0.0f, quakeIntensityXY.y), 0.0f };
					const glm::vec3 p4 = { p2.x, p3.y, 0.0f };

					return std::array<glm::vec3, 6>{ p1, p2, p3, p3, p2, p4 };
				});
			glEnable(GL_BLEND);
		};
	}

	template <typename ShadersPrograms>
	auto Demo3DRotatedFullscreenRenderer(ShadersPrograms& shadersProgram)
	{
		const auto& screenInfo = Globals::Components().screenInfo();

		return[&, angle = 0.0f](unsigned textureId) mutable
		{
			glm::mat4 vp = glm::perspective(glm::radians(28.0f), (float)screenInfo.windowSize.x / screenInfo.windowSize.y, 1.0f, 10.0f);
			glm::mat4 model = glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, -4.0f });
			model = glm::rotate(model, angle, { 1.0f, 1.0f, 1.0f });
			model = glm::scale(model, { (float)screenInfo.windowSize.x / screenInfo.windowSize.y, 1.0f, 1.0f });

			const float angleDelta = Globals::Components().physics().frameDuration * 2.0f;

			if (angle == 0.0f && Globals::Components().mouseState().xmb2)
			{
				angle = angleDelta;
			}
			if (angle > 0.0f && angle <= glm::two_pi<float>())
			{
				angle += angleDelta;
			}
			if (angle > glm::two_pi<float>())
			{
				angle = 0.0f;
			}

			glDisable(GL_BLEND);
			Tools::TexturedScreenRender(shadersProgram, textureId, [&]()
				{
					shadersProgram.vpUniform(vp);
					shadersProgram.modelUniform(model);
				}, [&]()
				{
					const float quakeIntensity = 0.001f * Globals::Components().shockwaves().size();
					const glm::vec2 quakeIntensityXY = screenInfo.windowSize.x > screenInfo.windowSize.y
						? glm::vec2(quakeIntensity, quakeIntensity * (float)screenInfo.windowSize.x / screenInfo.windowSize.y)
						: glm::vec2(quakeIntensity * (float)screenInfo.windowSize.y / screenInfo.windowSize.x, quakeIntensity);

					const glm::vec3 p1 = { -1.0f - Tools::Random(0.0f, quakeIntensityXY.x), -1.0f - Tools::Random(0.0f, quakeIntensityXY.y), 0.0f };
					const glm::vec3 p2 = { 1.0f + Tools::Random(0.0f, quakeIntensityXY.x), p1.y, 0.0f };
					const glm::vec3 p3 = { p1.x, 1.0f + Tools::Random(0.0f, quakeIntensityXY.y), 0.0f };
					const glm::vec3 p4 = { p2.x, p3.y, 0.0f };

					return std::array<glm::vec3, 6>{ p1, p2, p3, p3, p2, p4 };
				});
			glEnable(GL_BLEND);
		};
	}
}
