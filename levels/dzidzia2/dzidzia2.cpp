#include "dzidzia2.hpp"

#include <components/graphicsSettings.hpp>
#include <components/systemInfo.hpp>
#include <components/physics.hpp>
#include <components/texture.hpp>
#include <components/decoration.hpp>
#include <components/keyboard.hpp>
#include <components/mouse.hpp>

#include <systems/decorations.hpp>

#include <globals/components.hpp>
#include <globals/systems.hpp>

#include <ogl/renderingHelpers.hpp>
#include <ogl/shaders/textured.hpp>

#include <tools/Shapes2D.hpp>
#include <tools/gameHelpers.hpp>

namespace
{
	constexpr float turningSensitivity = 0.01f;
	unsigned dzidziaTailSize = 100;
}

namespace Levels
{
	class Dzidzia2::Impl
	{
	public:
		void setGraphicsSettings() const
		{
			Globals::Components().graphicsSettings().backgroundColorF = glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
		}

		void loadTextures()
		{
			auto& textures = Globals::Components().staticTextures();

			dzidzia1Texture = textures.size();
			textures.emplace("textures/photos/dzidzia1.png");
			textures.last().scale = glm::vec2(1.0f, 1.0f);
			textures.last().minFilter = GL_LINEAR;
			textures.last().preserveAspectRatio = true;

			dzidziaITata1Texture = textures.size();
			textures.emplace("textures/photos/dzidzia i tata 1.png");
			textures.last().scale = glm::vec2(1.0f, 1.0f);
			textures.last().minFilter = GL_LINEAR;
			textures.last().preserveAspectRatio = true;
		}

		void createBackground()
		{
			Tools::CreateJuliaBackground(Tools::JuliaParams{}.juliaCOffsetF([this]() {
				return Globals::Components().staticDecorations()[dzidziaDecorationId].originF() * 0.0001f; }));
		}

		void createDecorations()
		{
			auto& staticDecorations = Globals::Components().staticDecorations();
			const auto& physics = Globals::Components().physics();

			auto pos = std::make_shared<glm::vec2>(0.0f);

			auto renderingSetupF = [
				visibilityReduction = UniformsUtils::Uniform1b(),
				visibilityCenter = UniformsUtils::Uniform2f(),
				fullVisibilityDistance = UniformsUtils::Uniform1f(),
				invisibilityDistance = UniformsUtils::Uniform1f(),
				pos
			](ShadersUtils::ProgramId program) mutable {
				if (!visibilityReduction.isValid())
				{
					visibilityReduction = UniformsUtils::Uniform1b(program, "visibilityReduction");
					visibilityCenter = UniformsUtils::Uniform2f(program, "visibilityCenter");
					fullVisibilityDistance = UniformsUtils::Uniform1f(program, "fullVisibilityDistance");
					invisibilityDistance = UniformsUtils::Uniform1f(program, "invisibilityDistance");
				}

				visibilityReduction(true);
				visibilityCenter(*pos);
				fullVisibilityDistance(4.5f);
				invisibilityDistance(5.0f);

				return [=]() mutable {
					visibilityReduction(false);
				};
			};

			staticDecorations.emplace(Tools::Shapes2D::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 5.0f, 5.0f }),
				CM::Texture(dzidziaITata1Texture, true), Tools::Shapes2D::CreateTexCoordOfRectangle(), std::move(renderingSetupF));
			staticDecorations.last().modelMatrixF = [pos, step = glm::vec2(5.0f)]() mutable {
				const auto& screenInfo = Globals::Components().systemInfo().screen;
				const glm::vec2 absClamp = { screenInfo.getAspectRatio() * 10.0f, 10.0f };

				*pos += step * Globals::Components().physics().frameDuration;
				for (int i : {0, 1})
					if ((*pos)[i] < -absClamp[i] || (*pos)[i] > absClamp[i])
						step[i] = -step[i];

				return glm::translate(glm::mat4(1.0f), glm::vec3(*pos, 0.0f));
			};

			dzidziaDecorationId = staticDecorations.size();
			auto& dzidzia = staticDecorations.emplace(Tools::Shapes2D::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 4.0f, 4.0f }),
				CM::Texture(dzidzia1Texture, true), Tools::Shapes2D::CreateTexCoordOfRectangle());
			dzidzia.modelMatrixF = [this]() mutable {
				return glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(mousePos, 0.0f)), rotateAngle, { 0, 0, -1 }), glm::vec3((glm::sin(scaleSin) + 1.0f) / 2.0f));
			};
			dzidzia.posInSubsequence = dzidziaTailSize;
			for (unsigned i = 0; i < dzidziaTailSize; ++i)
			{
				dzidzia.subsequence.emplace_back(dzidzia.positions, dzidzia.texCoord, dzidzia.texture);
				dzidzia.subsequence.back().modelMatrixF = [pos = this->mousePos, angle = this->rotateAngle, scaleSin = this->scaleSin]() {
					return glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f)), angle, { 0, 0, -1 }), glm::vec3((glm::sin(scaleSin) + 1.0f) / 2.0f));
				};
				dzidzia.subsequence.back().colorF = [&, alpha = (float)i / dzidziaTailSize, deltaAlpha = 1.0f / dzidziaTailSize]() mutable {
					if (alpha < 0.000001 && !physics.paused)
						alpha = 1.0f;
					alpha -= deltaAlpha * !physics.paused;
					return glm::vec4(alpha);
				};
			}
		}

		void step()
		{
			const auto& mouse = Globals::Components().mouse();
			const auto& screenInfo = Globals::Components().systemInfo().screen;
			const auto& physics = Globals::Components().physics();
			auto& staticDecorations = Globals::Components().staticDecorations();

			absClamp = { screenInfo.getAspectRatio() * 10.0f, 10.0f };
			mousePos += mouse.getCartesianDelta() * turningSensitivity;
			mousePos = glm::clamp(mousePos, -absClamp, absClamp);

			auto& dzidzia = Globals::Components().staticDecorations()[dzidziaDecorationId];

			dzidzia.subsequence[dzidzia.subsequenceBegin].modelMatrixF = [pos = this->mousePos, angle = this->rotateAngle, scaleSin = this->scaleSin]() {
				return glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f)), angle, { 0, 0, -1 }), glm::vec3((glm::sin(scaleSin) + 1.0f) / 2.0f));
			};
			++dzidzia.subsequenceBegin %= dzidzia.subsequence.size();

			if (mouse.delta != glm::ivec2(0))
			{
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
