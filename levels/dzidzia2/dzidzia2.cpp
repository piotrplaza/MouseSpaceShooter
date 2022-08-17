#include "dzidzia2.hpp"

#include <components/graphicsSettings.hpp>
#include <components/screenInfo.hpp>
#include <components/physics.hpp>
#include <components/texture.hpp>
#include <components/decoration.hpp>
#include <components/keyboard.hpp>
#include <components/mouse.hpp>
#include <components/renderingSetup.hpp>

#include <systems/decorations.hpp>

#include <globals/components.hpp>
#include <globals/systems.hpp>

#include <ogl/renderingHelpers.hpp>
#include <ogl/shaders/textured.hpp>

#include <tools/graphicsHelpers.hpp>
#include <tools/gameHelpers.hpp>

namespace
{
	constexpr float mouseSensitivity = 0.01f;
	unsigned dzidziaTailSize = 200;
}

namespace Levels
{
	class Dzidzia2::Impl
	{
	public:
		void setGraphicsSettings() const
		{
			Globals::Components().graphicsSettings().clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().textures();

			dzidzia1Texture = textures.size();
			textures.emplace_back("textures/photos/dzidzia1.png");
			textures.back().scale = glm::vec2(1.0f, 1.0f);
			textures.back().minFilter = GL_LINEAR;

			dzidziaITata1Texture = textures.size();
			textures.emplace_back("textures/photos/dzidzia i tata 1.png");
			textures.back().scale = glm::vec2(1.0f, 1.0f);
			textures.back().minFilter = GL_LINEAR;
		}

		void createBackground()
		{
			Tools::CreateJuliaBackground([this]() {
				return Globals::Components().decorations()[dzidziaDecorationId].getCenter() * 0.0001f; });
		}

		void createDecorations()
		{
			auto& decorations = Globals::Components().decorations();
			auto& renderingSetups = Globals::Components().renderingSetups();

			auto pos = std::make_shared<glm::vec2>(0.0f);

			renderingSetups.emplace_back([
				visibilityReduction = Uniforms::Uniform1b(),
				visibilityCenter = Uniforms::Uniform2f(),
				fullVisibilityDistance = Uniforms::Uniform1f(),
				invisibilityDistance = Uniforms::Uniform1f(),
				pos
			](Shaders::ProgramId program) mutable {
				if (!visibilityReduction.isValid())
				{
					visibilityReduction = Uniforms::Uniform1b(program, "visibilityReduction");
					visibilityCenter = Uniforms::Uniform2f(program, "visibilityCenter");
					fullVisibilityDistance = Uniforms::Uniform1f(program, "fullVisibilityDistance");
					invisibilityDistance = Uniforms::Uniform1f(program, "invisibilityDistance");
				}

				visibilityReduction(true);
				visibilityCenter(*pos);
				fullVisibilityDistance(4.5f);
				invisibilityDistance(5.0f);

				return [=]() mutable {
					visibilityReduction(false);
				};
			});

			decorations.emplace_back(Tools::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 5.0f, 5.0f }),
				TCM::Texture(dzidziaITata1Texture), Tools::CreateTexCoordOfRectangle(), renderingSetups.size() - 1).preserveTextureRatio = true;
			decorations.back().modelMatrixF = [pos, step = glm::vec2(5.0f)]() mutable {
				const auto& screenInfo = Globals::Components().screenInfo();
				const glm::vec2 absClamp = { (float)screenInfo.windowSize.x / screenInfo.windowSize.y * 10.0f, 10.0f };

				*pos += step * Globals::Components().physics().frameDuration;
				for (int i : {0, 1})
					if ((*pos)[i] < -absClamp[i] || (*pos)[i] > absClamp[i])
						step[i] = -step[i];

				return glm::translate(glm::mat4(1.0f), glm::vec3(*pos, 0.0f));
			};

			dzidziaDecorationId = decorations.size();
			auto& dzidzia = decorations.emplace_back(Tools::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 4.0f, 4.0f }),
				TCM::Texture(dzidzia1Texture), Tools::CreateTexCoordOfRectangle());
			dzidzia.preserveTextureRatio = true;
			dzidzia.modelMatrixF = [this]() mutable {
				return glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(mousePos, 0.0f)), rotateAngle, { 0, 0, -1 }), glm::vec3((glm::sin(scaleSin) + 1.0f) / 2.0f));
			};
			dzidzia.posInSubsequence = dzidziaTailSize;
			dzidzia.subsequence.reserve(dzidziaTailSize);
			for (unsigned i = 0; i < dzidziaTailSize; ++i)
			{
				dzidzia.subsequence.emplace_back(dzidzia.vertices, dzidzia.texture, dzidzia.texCoord);
				dzidzia.subsequence.back().preserveTextureRatio = dzidzia.preserveTextureRatio;
				dzidzia.subsequence.back().modelMatrixF = [pos = this->mousePos, angle = this->rotateAngle, scaleSin = this->scaleSin]() {
					return glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f)), angle, { 0, 0, -1 }), glm::vec3((glm::sin(scaleSin) + 1.0f) / 2.0f));
				};
				dzidzia.subsequence.back().colorF = []() {
					return glm::vec4(0.2f);
				};
			}
		}

		void step()
		{
			const auto& mouse = Globals::Components().mouse();
			const auto& screenInfo = Globals::Components().screenInfo();
			const auto& physics = Globals::Components().physics();
			auto& decorations = Globals::Components().decorations();

			absClamp = { (float)screenInfo.windowSize.x / screenInfo.windowSize.y * 10.0f, 10.0f };
			mousePos += mouse.getWorldSpaceDelta() * mouseSensitivity;
			mousePos = glm::clamp(mousePos, -absClamp, absClamp);

			if (mouse.delta != glm::ivec2(0))
			{
				auto& dzidzia = Globals::Components().decorations()[dzidziaDecorationId];

				dzidzia.subsequence[dzidzia.subsequenceBegin].modelMatrixF = [pos = this->mousePos, angle = this->rotateAngle, scaleSin = this->scaleSin]() {
					return glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f)), angle, { 0, 0, -1 }), glm::vec3((glm::sin(scaleSin) + 1.0f) / 2.0f));
				};
				++dzidzia.subsequenceBegin %= dzidzia.subsequence.size();

				rotateAngle += 2.0f * physics.frameDuration;
				scaleSin += 2.0f * physics.frameDuration;
			}
		}

	private:
		unsigned dzidzia1Texture = 0;
		unsigned dzidziaITata1Texture = 0;

		unsigned dzidziaDecorationId = 0;

		glm::vec2 absClamp{ 0.0f };
		glm::vec2 mousePos{ 0.0f };

		float rotateAngle = 0.0f;
		float scaleSin = 0.0f;
	};

	Dzidzia2::Dzidzia2():
		impl(std::make_unique<Impl>())
	{
		impl->setGraphicsSettings();
		impl->loadTextures();
		impl->createBackground();
		impl->createDecorations();
	}

	Dzidzia2::~Dzidzia2() = default;

	void Dzidzia2::step()
	{
		impl->step();
	}
}
