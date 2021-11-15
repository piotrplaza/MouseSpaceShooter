#pragma once

#include <functional>
#include <optional>
#include <array>
#include <variant>

#include <GL/glew.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <globals.hpp>
#include <components/texture.hpp>
#include <components/animationTexture.hpp>
#include <components/mvp.hpp>
#include <components/screenInfo.hpp>
#include <components/physics.hpp>
#include <components/mouseState.hpp>

#include <ogl/oglProxy.hpp>

#include <tools/utility.hpp>

#include <commonTypes/typeComponentMappers.hpp>

namespace Tools
{
	template <typename ShadersProgram>
	inline void MVPInitialization(ShadersProgram& shadersProgram, std::optional<glm::mat4> modelMatrix = std::nullopt)
	{
		shadersProgram.vpUniform.setValue(Globals::Components().mvp().getVP());
		shadersProgram.modelUniform.setValue(modelMatrix ? *modelMatrix : glm::mat4(1.0f));
	}

	template <typename ShadersProgram>
	inline void StaticTexturedRenderInitialization(ShadersProgram& shadersProgram, unsigned textureId, bool textureRatioPreserved)
	{
		const auto& textureComponent = Globals::Components().textures()[textureId];

		shadersProgram.texture1Uniform.setValue(textureId);
		shadersProgram.textureTranslateUniform.setValue(textureComponent.translate);
		shadersProgram.textureScaleUniform.setValue(
			{ (textureRatioPreserved ? (float)textureComponent.loaded.size.x / textureComponent.loaded.size.y : 1.0f)
			* textureComponent.scale.x, textureComponent.scale.y });
	}

	template <typename ShadersProgram>
	inline void AnimatedTexturedRenderInitialization(ShadersProgram& shadersProgram, unsigned animationTextureId)
	{
		const auto& animationTexture = Globals::Components().animationTextures()[animationTextureId];

		shadersProgram.texture1Uniform.setValue(animationTexture.getTextureId());
		const auto frameTransformation = animationTexture.getFrameTransformation();
		shadersProgram.textureTranslateUniform.setValue(frameTransformation.translate);
		shadersProgram.textureScaleUniform.setValue(frameTransformation.scale);
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

		void operator ()(TCM::AnimationTexture animationTexture)
		{
			AnimatedTexturedRenderInitialization(shadersProgram, animationTexture.id);
		}

		void operator ()(TCM::BlendingTexture blendingTexture)
		{
			assert(!"Unsupported yet.");
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
			renderingTeardown = (*buffers.renderingSetup)(shadersProgram.getProgramId());

		glBindVertexArray(buffers.vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, buffers.positionsCache.size());
		
		if (renderingTeardown)
			renderingTeardown();
	}

	template <typename ShadersPrograms>
	inline void TexturedScreenRender(ShadersPrograms& shadersProgram, unsigned texture, std::function<void()> customSetup = nullptr, std::function<std::array<glm::vec3, 6>()> customSize = nullptr)
	{
		const int numOfVertices = 6;

		const static glm::vec3 positions[numOfVertices] =
		{
			{-1.0f, -1.0f, 0.0f},
			{1.0f, -1.0f, 0.0f},
			{-1.0f, 1.0f, 0.0f},
			{-1.0f, 1.0f, 0.0f},
			{1.0f, -1.0f, 0.0f},
			{1.0f, 1.0f, 0.0f}
		};

		const static glm::vec2 texCoords[numOfVertices] =
		{
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{0.0f, 1.0f},
			{0.0f, 1.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f}
		};

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		if (customSize)
			glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, &customSize()[0]);
		else
			glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, &positions);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, &texCoords);
		glEnableVertexAttribArray(1);

		glUseProgram_proxy(shadersProgram.getProgramId());

		shadersProgram.modelUniform.setValue(glm::mat4(1.0f));
		shadersProgram.vpUniform.setValue(glm::mat4(1.0f));
		shadersProgram.colorUniform.setValue(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		shadersProgram.textureTranslateUniform.setValue(glm::vec2(0.0f));
		shadersProgram.textureScaleUniform.setValue(glm::vec2(1.0f));
		shadersProgram.texture1Uniform.setValue(texture);

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
					shadersProgram.vpUniform.setValue(vp);
					shadersProgram.modelUniform.setValue(model);
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
