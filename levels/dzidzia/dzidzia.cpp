#include "dzidzia.hpp"

#include <components/graphicsSettings.hpp>
#include <components/screenInfo.hpp>
#include <components/physics.hpp>
#include <components/texture.hpp>
#include <components/decoration.hpp>
#include <components/keyboardState.hpp>
#include <components/mouseState.hpp>

#include <systems/decorations.hpp>

#include <globals/components.hpp>
#include <globals/systems.hpp>

#include <ogl/renderingHelpers.hpp>
#include <ogl/shaders/textured.hpp>

#include <tools/graphicsHelpers.hpp>

namespace Levels
{
	class Dzidzia::Impl
	{
	public:
		void reserveMemory() const
		{
			Globals::Components().decorations().reserve(1000000);
		}

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

		void createDecorations()
		{
			auto& decorations = Globals::Components().decorations();

			decorations.emplace_back(Tools::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 5.0f, 5.0f }),
				TCM::Texture(dzidziaITata1Texture), Tools::CreateTexCoordOfRectangle()).preserveTextureRatio = true;
			decorations.back().modelMatrixF = [pos = glm::vec2(0.0f), step = glm::vec2(10.0f)]() mutable {
				const auto& screenInfo = Globals::Components().screenInfo();
				const glm::vec2 absClamp = { (float)screenInfo.windowSize.x / screenInfo.windowSize.y * 10.0f, 10.0f };

				pos += step * Globals::Components().physics().frameDuration;
				for (int i : {0, 1})
					if (pos[i] < -absClamp[i] || pos[i] > absClamp[i])
						step[i] = -step[i];

				return glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
			};

			decorations.emplace_back(Tools::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 4.0f, 4.0f }),
				TCM::Texture(dzidzia1Texture), Tools::CreateTexCoordOfRectangle()).preserveTextureRatio = true;
			decorations.back().modelMatrixF = [this]() mutable {
				return glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(mousePos, 0.0f)), rotateAngle, { 0, 0, -1 }), glm::vec3((glm::sin(scaleSin) + 1.0f) / 2.0f));
			};
		}

		void step()
		{
			const auto& keyboardState = Globals::Components().keyboardState();
			const auto& mouseState = Globals::Components().mouseState();
			const auto& screenInfo = Globals::Components().screenInfo();
			const auto& physics = Globals::Components().physics();
			auto& decorations = Globals::Components().decorations();

			absClamp = { (float)screenInfo.windowSize.x / screenInfo.windowSize.y * 10.0f, 10.0f };
			mousePos += mouseState.getWorldSpaceDelta() * 0.01f;
			mousePos = glm::clamp(mousePos, -absClamp, absClamp);
			
			if (keyboardState.pressed[' '])
				draw = !draw;

			if (draw)
			{
				if (mouseState.delta != glm::ivec2(0))
				{
					decorations.back().modelMatrixF = [pos = this->mousePos, angle = this->rotateAngle, scaleSin = this->scaleSin]() {
						return glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f)), angle, { 0, 0, -1 }), glm::vec3((glm::sin(scaleSin) + 1.0f) / 2.0f));
					};

					assert(decorations.size() < decorations.capacity());
					decorations.emplace_back(Tools::CreateVerticesOfRectangle({ 0.0f, 0.0f }, { 4.0f, 4.0f }),
						TCM::Texture(dzidzia1Texture), Tools::CreateTexCoordOfRectangle()).preserveTextureRatio = true;
					decorations.back().modelMatrixF = [this]() mutable {
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

		glm::vec2 absClamp{ 0.0f };
		glm::vec2 mousePos{ 0.0f };

		bool draw = false;

		float rotateAngle = 0.0f;
		float scaleSin = 0.0f;
	};

	Dzidzia::Dzidzia():
		impl(std::make_unique<Impl>())
	{
		impl->reserveMemory();
		impl->setGraphicsSettings();
		impl->loadTextures();
		impl->createDecorations();
	}

	Dzidzia::~Dzidzia() = default;

	void Dzidzia::step()
	{
		impl->step();
	}
}
