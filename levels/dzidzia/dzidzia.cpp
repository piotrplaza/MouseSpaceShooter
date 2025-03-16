#include "dzidzia.hpp"

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
}

namespace Levels
{
	class Dzidzia::Impl
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
				return Globals::Components().staticDecorations()[dzidziaDecoration].originF() * 0.0001f; }));
		}

		void createDecorations()
		{
			auto& staticDecorations = Globals::Components().staticDecorations();

			auto pos = std::make_shared<glm::vec2>(0.0f);

			auto renderigSetupF = [
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
				CM::Texture(dzidziaITata1Texture, true), Tools::Shapes2D::CreateTexCoordOfRectangle(), std::move(renderigSetupF));
			staticDecorations.last().modelMatrixF = [pos, step = glm::vec2(5.0f)]() mutable {
				const auto& screenInfo = Globals::Components().systemInfo().screen;
				const glm::vec2 absClamp = { screenInfo.getAspectRatio() * 10.0f, 10.0f };

				*pos += step * Globals::Components().physics().frameDuration;
				for (int i : {0, 1})
					if ((*pos)[i] < -absClamp[i] || (*pos)[i] > absClamp[i])
						step[i] = -step[i];

				return glm::translate(glm::mat4(1.0f), glm::vec3(*pos, 0.0f));
			};

			dzidziaDecoration = staticDecorations.size();
			staticDecorations.emplace(Tools::Shapes2D::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 4.0f, 4.0f }),
				CM::Texture(dzidzia1Texture, true), Tools::Shapes2D::CreateTexCoordOfRectangle());
			staticDecorations.last().modelMatrixF = [this]() mutable {
				return glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(mousePos, 0.0f)), rotateAngle, { 0, 0, -1 }), glm::vec3((glm::sin(scaleSin) + 1.0f) / 2.0f));
			};
		}

		void step()
		{
			const auto& keyboard = Globals::Components().keyboard();
			const auto& mouse = Globals::Components().mouse();
			const auto& screenInfo = Globals::Components().systemInfo().screen;
			const auto& physics = Globals::Components().physics();
			auto& staticDecorations = Globals::Components().staticDecorations();

			absClamp = { screenInfo.getAspectRatio() * 10.0f, 10.0f };
			mousePos += mouse.getCartesianDelta() * turningSensitivity;
			mousePos = glm::clamp(mousePos, -absClamp, absClamp);
			
			if (keyboard.pressed[' '] || mouse.pressed.lmb)
				draw = !draw;

			if (draw)
			{
				if (mouse.delta != glm::ivec2(0))
				{
					staticDecorations.last().modelMatrixF = [pos = this->mousePos, angle = this->rotateAngle, scaleSin = this->scaleSin]() {
						return glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f)), angle, { 0, 0, -1 }), glm::vec3((glm::sin(scaleSin) + 1.0f) / 2.0f));
					};

					dzidziaDecoration = staticDecorations.size();
					staticDecorations.emplace(Tools::Shapes2D::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 4.0f, 4.0f }),
						CM::Texture(dzidzia1Texture, true), Tools::Shapes2D::CreateTexCoordOfRectangle());
					staticDecorations.last().modelMatrixF = [this]() mutable {
						return glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(mousePos, 0.0f)), rotateAngle, { 0, 0, -1 }), glm::vec3((glm::sin(scaleSin) + 1.0f) / 2.0f));
					};
					Globals::Systems().decorations().updateStaticBuffers();

					rotateAngle += 2.0f * physics.frameDuration;
					scaleSin += 2.0f * physics.frameDuration;
				}
			}
		}

	private:
		unsigned dzidzia1Texture = 0;
		unsigned dzidziaITata1Texture = 0;

		unsigned dzidziaDecoration = 0;

		glm::vec2 absClamp{ 0.0f };
		glm::vec2 mousePos{ 0.0f };

		bool draw = false;

		float rotateAngle = 0.0f;
		float scaleSin = 0.0f;
	};

	Dzidzia::Dzidzia():
		impl(std::make_unique<Impl>())
	{
		impl->setGraphicsSettings();
		impl->loadTextures();
		impl->createBackground();
		impl->createDecorations();
	}

	Dzidzia::~Dzidzia() = default;

	void Dzidzia::step()
	{
		impl->step();
	}
}
