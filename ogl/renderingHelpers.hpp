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

#include <commonTypes/componentMappers.hpp>

#include <globals/components.hpp>

#include <ogl/oglProxy.hpp>

#include <tools/utility.hpp>

#include <functional>
#include <optional>
#include <array>
#include <variant>

namespace
{
	template <typename ShadersProgram>
	inline void PrepareTexturedRender(ShadersProgram& shadersProgram, const AbstractTextureComponentVariant& texture, bool preserveTextureRatio,
		const glm::mat4& additionalTransform, unsigned textureId);

	inline void TexturedRenderInitialization(auto& shadersProgram, const Components::Texture& textureComponent, bool preserveTextureRatio,
		glm::vec2 translate, float rotate, glm::vec2 scale, const glm::mat4& additionalTransform, unsigned textureId)
	{
		if (textureId == 0)
			shadersProgram.numOfTextures(1);

		shadersProgram.textures(textureId, textureComponent.loaded.textureUnit - GL_TEXTURE0);
		shadersProgram.texturesBaseTransform(textureId, Tools::TextureTransform(textureComponent, preserveTextureRatio)
			* Tools::TextureTransform(translate, rotate, scale) * additionalTransform);
	}

	inline void AnimatedTexturedRenderInitialization(auto& shadersProgram, const Components::AnimatedTexture& animatedTextureComponent,
		glm::vec2 translate, float rotate, glm::vec2 scale, const glm::mat4& additionalTransform, unsigned textureId)
	{
		struct TextureComponentSelectorVisitor
		{
			const Components::Texture& operator ()(const CM::StaticTexture& texture) const
			{
				return *texture.component;
			}

			const Components::Texture& operator ()(const CM::DynamicTexture& texture) const
			{
				return *texture.component;
			}

			const Components::Texture& operator ()(std::monostate) const
			{
				throw std::runtime_error("TextureComponentSelectorVisitor: std::monostate not allowed.");
			}
		};

		if (textureId == 0)
			shadersProgram.numOfTextures(1);

		const Components::Texture& textureComponent = std::visit(TextureComponentSelectorVisitor{}, animatedTextureComponent.getTexture());
		shadersProgram.textures(textureId, textureComponent.loaded.textureUnit - GL_TEXTURE0);
		shadersProgram.texturesBaseTransform(textureId, animatedTextureComponent.getFrameTransformation() * Tools::TextureTransform(translate, rotate, scale)
			* additionalTransform);
	}

	inline void BlendingTexturedRenderInitialization(auto& shadersProgram, const Components::BlendingTexture& blendingTextureComponent,
		bool preserveTextureRatio, glm::vec2 translate, float rotate, glm::vec2 scale)
	{
		for (unsigned i = 0; i < (unsigned)blendingTextureComponent.textures.size(); ++i)
			PrepareTexturedRender(shadersProgram, blendingTextureComponent.textures[i], preserveTextureRatio, Tools::TextureTransform(translate, rotate, scale), i);

		shadersProgram.numOfTextures(blendingTextureComponent.textures.size());
	}

	template <typename ShadersProgram>
	inline void PrepareTexturedRender(ShadersProgram& shadersProgram, const AbstractTextureComponentVariant& texture, bool preserveTextureRatio,
		const glm::mat4& additionalTransform, unsigned textureId)
	{
		class AbstractTexturedRenderInitializationVisitor
		{
		public:
			AbstractTexturedRenderInitializationVisitor(ShadersProgram& shadersProgram, bool preserveTextureRatio,
				const glm::mat4& additionalTransform, unsigned textureId) :
				shadersProgram(shadersProgram),
				preserveTextureRatio(preserveTextureRatio),
				additionalTransform(additionalTransform),
				textureId(textureId)
			{
			}

			void operator ()(const CM::StaticTexture& texture) const
			{
				TexturedRenderInitialization(shadersProgram, *texture.component, preserveTextureRatio, texture.translate, texture.rotate, texture.scale,
					additionalTransform, textureId);
			}

			void operator ()(const CM::DynamicTexture& texture) const
			{
				TexturedRenderInitialization(shadersProgram, *texture.component, preserveTextureRatio, texture.translate, texture.rotate, texture.scale,
					additionalTransform, textureId);
			}

			void operator ()(const CM::StaticAnimatedTexture& animatedTexture) const
			{
				AnimatedTexturedRenderInitialization(shadersProgram, *animatedTexture.component, animatedTexture.translate, animatedTexture.rotate, animatedTexture.scale,
					additionalTransform, textureId);
			}

			void operator ()(const CM::DynamicAnimatedTexture& animatedTexture) const
			{
				AnimatedTexturedRenderInitialization(shadersProgram, *animatedTexture.component, animatedTexture.translate, animatedTexture.rotate, animatedTexture.scale,
					additionalTransform, textureId);
			}

			void operator ()(const CM::StaticBlendingTexture& blendingTexture) const
			{
				BlendingTexturedRenderInitialization(shadersProgram, *blendingTexture.component, preserveTextureRatio, blendingTexture.translate, blendingTexture.rotate, blendingTexture.scale);
			}

			void operator ()(const CM::DynamicBlendingTexture& blendingTexture) const
			{
				BlendingTexturedRenderInitialization(shadersProgram, *blendingTexture.component, preserveTextureRatio, blendingTexture.translate, blendingTexture.rotate, blendingTexture.scale);
			}

			void operator ()(std::monostate) const
			{
				shadersProgram.numOfTextures(0);
			}

		private:
			ShadersProgram& shadersProgram;
			bool preserveTextureRatio;
			const glm::mat4& additionalTransform;
			unsigned textureId;
		};

		std::visit(AbstractTexturedRenderInitializationVisitor{ shadersProgram, preserveTextureRatio, additionalTransform, textureId }, texture);
	}
}

namespace Tools
{
	inline void PrepareTexturedRender(auto& shadersProgram, const AbstractTextureComponentVariant& texture, bool preserveTextureRatio,
		const glm::mat4& additionalTransform = glm::mat4(1.0f), unsigned textureId = 0)
	{
		::PrepareTexturedRender(shadersProgram, texture, preserveTextureRatio, additionalTransform, textureId);
	}

	inline void MVPInitialization(auto& shadersProgram, std::optional<glm::mat4> modelMatrix = std::nullopt)
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

	inline void TexturedScreenRender(auto& shadersProgram, unsigned texture, std::function<void()> customSetup = nullptr,
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

	inline auto StandardFullscreenRenderer(auto& shadersProgram)
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

					const glm::vec3 p1 = { -1.0f - Tools::RandomFloat(0.0f, quakeIntensityXY.x), -1.0f - Tools::RandomFloat(0.0f, quakeIntensityXY.y), 0.0f };
					const glm::vec3 p2 = { 1.0f + Tools::RandomFloat(0.0f, quakeIntensityXY.x), p1.y, 0.0f };
					const glm::vec3 p3 = { p1.x, 1.0f + Tools::RandomFloat(0.0f, quakeIntensityXY.y), 0.0f };
					const glm::vec3 p4 = { p2.x, p3.y, 0.0f };

					return std::array<glm::vec3, 6>{ p1, p2, p3, p3, p2, p4 };
				});
			glEnable(GL_BLEND);
		};
	}

	inline auto Demo3DRotatedFullscreenRenderer(auto& shadersProgram)
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

			const bool blendEnabled = glIsEnabled(GL_BLEND);
			const bool cullFaceEnabled = glIsEnabled(GL_CULL_FACE);

			if (blendEnabled)
				glDisable(GL_BLEND);
			if (cullFaceEnabled)
				glDisable(GL_CULL_FACE);

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

					const glm::vec3 p1 = { -1.0f - Tools::RandomFloat(0.0f, quakeIntensityXY.x), -1.0f - Tools::RandomFloat(0.0f, quakeIntensityXY.y), 0.0f };
					const glm::vec3 p2 = { 1.0f + Tools::RandomFloat(0.0f, quakeIntensityXY.x), p1.y, 0.0f };
					const glm::vec3 p3 = { p1.x, 1.0f + Tools::RandomFloat(0.0f, quakeIntensityXY.y), 0.0f };
					const glm::vec3 p4 = { p2.x, p3.y, 0.0f };

					return std::array<glm::vec3, 6>{ p1, p2, p3, p3, p2, p4 };
				});

			if (blendEnabled)
				glEnable(GL_BLEND);
			if (cullFaceEnabled)
				glEnable(GL_CULL_FACE);
		};
	}

	inline void Lights3DSetup(auto& shadersProgram)
	{
		const auto& lights3D = Globals::Components().lights3D();
		const auto& mvp3D = Globals::Components().mvp3D();
		const auto& graphicsSettings = Globals::Components().graphicsSettings();

		shadersProgram.clearColor(graphicsSettings.clearColorF());
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
