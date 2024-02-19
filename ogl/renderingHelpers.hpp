#pragma once

#include <components/texture.hpp>
#include <components/blendingTexture.hpp>
#include <components/animatedTexture.hpp>
#include <components/mvp.hpp>
#include <components/screenInfo.hpp>
#include <components/physics.hpp>
#include <components/mouse.hpp>
#include <components/shockwave.hpp>
#include <components/light3D.hpp>
#include <components/graphicsSettings.hpp>

#include <commonTypes/typeComponentMappers.hpp>

#include <globals/components.hpp>

#include <ogl/oglProxy.hpp>

#include <tools/utility.hpp>

#include <GL/glew.h>

#include <functional>
#include <optional>
#include <array>
#include <variant>

namespace Tools
{
	template <typename ShadersProgram>
	inline void MVPInitialization(ShadersProgram& shadersProgram, std::optional<glm::mat4> modelMatrix = std::nullopt)
	{
		const auto modelMatrix_ = modelMatrix ? *modelMatrix : glm::mat4(1.0f);

		if constexpr (requires { shadersProgram.model; })
			shadersProgram.model(modelMatrix_);
		if constexpr (requires { shadersProgram.mv; })
			shadersProgram.mv(Globals::Components().mvp2D().getMV(modelMatrix_));
		if constexpr (requires { shadersProgram.vp; })
			shadersProgram.vp(Globals::Components().mvp2D().getVP());
		if constexpr (requires { shadersProgram.mvp; })
			shadersProgram.mvp(Globals::Components().mvp2D().getMVP(modelMatrix_));
		if constexpr (requires { shadersProgram.normalMatrix; })
			shadersProgram.normalMatrix(Globals::Components().mvp2D().getNormalMatrix());
	}

	template <typename ShadersProgram>
	inline void StaticTexturedRenderInitialization(ShadersProgram& shadersProgram, unsigned textureId, bool preserveTextureRatio,
		glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f })
	{
		const auto& textureComponent = Globals::Components().textures()[textureId];

		shadersProgram.numOfTextures(1);
		shadersProgram.textures(0, textureId);
		shadersProgram.texturesBaseTransform(0, TextureTransform(textureComponent, preserveTextureRatio) * TextureTransform(translate, rotate, scale));
	}

	template <typename ShadersProgram>
	inline void AnimatedTexturedRenderInitialization(ShadersProgram& shadersProgram, unsigned animatedTextureId,
		glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f })
	{
		auto& animationTextureComponent = Globals::Components().animatedTextures()[animatedTextureId];

		shadersProgram.numOfTextures(1);
		shadersProgram.textures(0, animationTextureComponent.getTextureId());
		shadersProgram.texturesBaseTransform(0, animationTextureComponent.getFrameTransformation() * TextureTransform(translate, rotate, scale));
	}

	template <typename ShadersProgram>
	inline void BlendingTexturedRenderInitialization(ShadersProgram& shadersProgram, unsigned blendingTextureId, bool preserveTextureRatio,
		glm::vec2 translate = { 0.0f, 0.0f }, float rotate = 0.0f, glm::vec2 scale = { 1.0f, 1.0f })
	{
		const auto& blendingTextureComponent = Globals::Components().blendingTextures()[blendingTextureId];

		unsigned iStart = 0;
		if (blendingTextureComponent.blendingAnimation)
		{
			iStart = 1;
			AnimatedTexturedRenderInitialization(shadersProgram, blendingTextureComponent.texturesIds[0], translate, rotate, scale);
		}

		shadersProgram.numOfTextures(blendingTextureComponent.texturesIds.size());
		for (unsigned i = iStart; i < (unsigned)blendingTextureComponent.texturesIds.size(); ++i)
		{
			const auto textureId = blendingTextureComponent.texturesIds[i];
			const auto& textureComponent = Globals::Components().textures()[textureId];

			shadersProgram.textures(i, textureId);
			shadersProgram.texturesBaseTransform(i, TextureTransform(textureComponent, preserveTextureRatio) * TextureTransform(translate, rotate, scale));
		}
	}

	template <typename ShadersProgram>
	class TexturedRenderInitializationVisitor
	{
	public:
		TexturedRenderInitializationVisitor(ShadersProgram& shadersProgram, bool preserveTextureRatio):
			shadersProgram(shadersProgram),
			preserveTextureRatio(preserveTextureRatio)
		{
		}

		void operator ()(const TCM::Texture& texture)
		{
			StaticTexturedRenderInitialization(shadersProgram, texture.id, preserveTextureRatio, texture.translate, texture.rotate, texture.scale);
		}

		void operator ()(const TCM::AnimatedTexture& animatedTexture)
		{
			AnimatedTexturedRenderInitialization(shadersProgram, animatedTexture.id, animatedTexture.translate, animatedTexture.rotate, animatedTexture.scale);
		}

		void operator ()(const TCM::BlendingTexture& blendingTexture)
		{
			BlendingTexturedRenderInitialization(shadersProgram, blendingTexture.id, preserveTextureRatio, blendingTexture.translate, blendingTexture.rotate, blendingTexture.scale);
		}

		void operator ()(std::monostate)
		{
			shadersProgram.numOfTextures(0);
		}

	private:
		ShadersProgram& shadersProgram;
		bool preserveTextureRatio;
	};

	template <typename ShadersProgram, typename Buffers>
	inline void PrepareTexturedRender(ShadersProgram& shadersProgram, const Buffers& buffers, const TextureComponentVariant& texture)
	{
		std::visit(TexturedRenderInitializationVisitor{ shadersProgram, buffers.renderable->preserveTextureRatio }, texture);
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
			{-0.5f, -0.5f},
			{0.5f, -0.5f},
			{-0.5f, 0.5f},
			{-0.5f, 0.5f},
			{0.5f, -0.5f},
			{0.5f, 0.5f}
		};

		static std::array<glm::vec3, numOfVertices> customPositions;

		glBindVertexArray_proxy(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		if (positionsGenerator)
		{
			customPositions = positionsGenerator();
			glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, &customPositions);
		}
		else
			glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, &defaultPositions);
		glEnableVertexAttribArray(0);

		glVertexAttrib4f(1, 1.0f, 1.0f, 1.0f, 1.0f);
		glDisableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, false, 0, &defaultTexCoords);
		glEnableVertexAttribArray(2);

		glUseProgram_proxy(shadersProgram.getProgramId());

		shadersProgram.model(glm::mat4(1.0f));
		shadersProgram.vp(glm::mat4(1.0f));
		shadersProgram.color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		shadersProgram.numOfTextures(1);
		shadersProgram.textures(0, texture);
		shadersProgram.texturesBaseTransform(0, glm::mat4(1.0f));

		if (customSetup)
			customSetup();

		glDrawArrays(GL_TRIANGLES, 0, numOfVertices);
	}

	template <typename ShadersPrograms>
	inline auto StandardFullscreenRenderer(ShadersPrograms& shadersProgram)
	{
		const auto& screenInfo = Globals::Components().screenInfo();

		return[&](unsigned textureId)
		{
			glDisable(GL_BLEND);
			TexturedScreenRender(shadersProgram, textureId, nullptr, [&]()
				{
					const float quakeIntensity = 0.001f * Globals::Components().shockwaves().size();
					const glm::vec2 quakeIntensityXY = screenInfo.windowSize.x > screenInfo.windowSize.y
						? glm::vec2(quakeIntensity, quakeIntensity * screenInfo.getAspectRatio())
						: glm::vec2(quakeIntensity / screenInfo.getAspectRatio(), quakeIntensity);

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
	inline auto Demo3DRotatedFullscreenRenderer(ShadersPrograms& shadersProgram)
	{
		const auto& screenInfo = Globals::Components().screenInfo();

		return[&, angle = 0.0f](unsigned textureId) mutable
		{
			glm::mat4 vp = glm::perspective(glm::radians(28.0f), screenInfo.getAspectRatio(), 1.0f, 10.0f);
			glm::mat4 model = glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, -4.0f });
			model = glm::rotate(model, angle, { 1.0f, 1.0f, 1.0f });
			model = glm::scale(model, { screenInfo.getAspectRatio(), 1.0f, 1.0f });

			const float angleDelta = Globals::Components().physics().frameDuration * 2.0f;

			if (angle == 0.0f && Globals::Components().mouse().pressing.xmb2)
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
					shadersProgram.vp(vp);
					shadersProgram.model(model);
				}, [&]()
				{
					const float quakeIntensity = 0.001f * Globals::Components().shockwaves().size();
					const glm::vec2 quakeIntensityXY = screenInfo.windowSize.x > screenInfo.windowSize.y
						? glm::vec2(quakeIntensity, quakeIntensity * screenInfo.getAspectRatio())
						: glm::vec2(quakeIntensity / screenInfo.getAspectRatio(), quakeIntensity);

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
	inline void Lights3DSetup(ShadersPrograms& shadersProgram)
	{
		const auto& lights3D = Globals::Components().lights3D();
		const auto& mvp3D = Globals::Components().mvp3D();
		const auto& graphicsSettings = Globals::Components().graphicsSettings();

		shadersProgram.clearColor(graphicsSettings.clearColor);
		shadersProgram.numOfLights(lights3D.size());
		unsigned i = 0;
		for (const auto& light : lights3D)
		{
			shadersProgram.lightsPos(i, (light.viewSpace ? glm::inverse(mvp3D.view) : glm::mat4(1.0f)) * glm::vec4(light.position, 1.0f));
			shadersProgram.lightsCol(i, light.color);
			shadersProgram.lightsAttenuation(i, light.attenuation);
			shadersProgram.clearColorFactor(i, light.clearColorFactor);
			++i;
		}
	}
}
