#include "paint.hpp"

#include <components/graphicsSettings.hpp>
#include <components/texture.hpp>
#include <components/decoration.hpp>
#include <components/camera2d.hpp>
#include <components/keyboard.hpp>
#include <components/mouse.hpp>

#include <globals/components.hpp>

#include <tools/shapes2D.hpp>
#include <tools/colorBufferEditor.hpp>
#include <tools/utility.hpp>

#include <execution>

namespace Levels
{
	class Paint::Impl
	{
	public:
		void setup()
		{
			Globals::Components().graphicsSettings().clearColor = { 0.0f, 0.1f, 0.1f, 1.0f };
			Globals::Components().camera2D().targetProjectionHSizeF = []() { return 0.5f; };
			auto& texture = Globals::Components().dynamicTextures().emplace(TextureData(TextureFile(texturePath, 3)));
			//texture.magFilter = GL_NEAREST;
			texture.wrapMode = GL_CLAMP_TO_EDGE;
			texture.sourceFragmentCornerAndSizeF = [marigin = 5](glm::ivec2 origSize) { return std::make_pair(glm::ivec2(marigin, marigin), origSize - glm::ivec2(marigin * 2)); };
			textureId = texture.getComponentId();

			auto& staticDecorations = Globals::Components().staticDecorations();
			staticDecorations.emplace(Shapes2D::CreateVerticesOfRectangle(), CM::DynamicTexture(textureId), Shapes2D::CreateTexCoordOfRectangle());
			auto& cursor = staticDecorations.emplace(Shapes2D::CreateVerticesOfCircle({ 0.0f, 0.0f }, 1.0f, 100));
			cursor.colorF = [&]() { return glm::vec4(cursorColor, 1.0f); };
			cursor.modelMatrixF = [&]() {
				cursorPos += Globals::Components().mouse().getCartesianDelta() * 0.0005f;
				cursorPos.x = std::clamp(cursorPos.x, -0.5f, 0.5f);
				cursorPos.y = std::clamp(cursorPos.y, -0.5f, 0.5f);
				return glm::translate(glm::mat4(1.0f), glm::vec3(cursorPos, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(cursorSize, cursorSize, 1.0f)
				);
			};
			cursor.renderLayer = RenderLayer::NearForeground;
		}

		void step()
		{
			const auto& keyboard = Globals::Components().keyboard();
			const auto& mouse = Globals::Components().mouse();
			auto& texture = Globals::Components().dynamicTextures()[textureId];
			auto& loadedTextureData = std::get<TextureData>(texture.source).loaded;
			auto& buffer = std::get<std::vector<glm::vec3>>(loadedTextureData.data);
			auto& textureSize = loadedTextureData.size;

			if (!editor)
				editor = std::make_unique<ColorBufferEditor>(buffer, textureSize);	

			switch (effect)
			{
			case 0: plasma(*editor); break;
			case 1: colorChanneling(*editor); break;
			case 2: flames(*editor); break;
			}

			if (mouse.pressing.lmb)
			{
				if constexpr (ColorBufferEditor::IsDoubleBuffering())
					editor->swapBuffers();

				const auto cursorPosInTexture = glm::ivec2((cursorPos + glm::vec2(0.5f)) * glm::vec2(textureSize - 1));
				//editor->putRectangle(cursorPosInTexture, { cursorSize * textureSize.x, cursorSize * textureSize.y }, cursorColor);
				//editor->putCircle(cursorPosInTexture, cursorSize * textureSize.y, cursorColor);
				editor->putEllipse(cursorPosInTexture, { cursorSize * textureSize.x, cursorSize * textureSize.y }, cursorColor);

				if constexpr (ColorBufferEditor::IsDoubleBuffering())
					editor->swapBuffers();
			}

			if (keyboard.pressed[(int)'1'])
				effect = 0;
			if (keyboard.pressed[(int)'2'])
				effect = 1;
			if (keyboard.pressed[(int)'3'])
				effect = 2;


			const float colorStep = 0.1f;
			if (keyboard.pressing[(int)'R'])
				cursorColor.r += mouse.pressed.wheel * colorStep;
			else if (keyboard.pressing[(int)'G'])
				cursorColor.g += mouse.pressed.wheel * colorStep;
			else if (keyboard.pressing[(int)'B'])
				cursorColor.b += mouse.pressed.wheel * colorStep;
			else
			{
				const float sizeStep = 0.05f;
				cursorSize += mouse.pressed.wheel * sizeStep * (cursorSize + 0.1f);
				cursorSize = std::clamp(cursorSize, 0.0f, 1.0f);
			}

			cursorColor.r = std::clamp(cursorColor.r, 0.0f, 1.0f);
			cursorColor.g = std::clamp(cursorColor.g, 0.0f, 1.0f);
			cursorColor.b = std::clamp(cursorColor.b, 0.0f, 1.0f);

			if (keyboard.pressing[0x20/*VK_SPACE*/])
			{
				auto& texture = Globals::Components().dynamicTextures()[textureId];
				texture.source = TextureData(TextureFile(texturePath, 3));
				editor.reset();
			}

			texture.state = ComponentState::Changed;
		}

	private:
		using ColorBufferEditor = Tools::ColorBufferEditor<glm::vec3, true>;

		void flames(auto& colorBuffer, float newColorFactor = 0.249f, glm::vec3 initRgbMin = glm::vec3(-200), glm::vec3 initRgbMax = glm::vec3(200))
		{
			for (int x = 0; x < colorBuffer.getRes().x; ++x)
				colorBuffer.putColor({ x, 0 }, { Tools::Random(initRgbMin.r, initRgbMax.r), Tools::Random(initRgbMin.g, initRgbMax.g), Tools::Random(initRgbMin.b, initRgbMax.b) });

			auto innerLoop = [&](const auto y) {
				for (int x = 0; x < colorBuffer.getRes().x; ++x)
				{
					glm::vec3 newColor = (colorBuffer.getColor({ x - 1, y - 1 }) + colorBuffer.getColor({ x, y - 1 }) + colorBuffer.getColor({ x + 1, y - 1 }) + colorBuffer.getColor({ x, y })) * newColorFactor;
					colorBuffer.putColor({ x, y }, newColor);
				}
			};

			if constexpr (ColorBufferEditor::IsDoubleBuffering())
			{
				Tools::ItToId itToId(1, colorBuffer.getRes().y);
				std::for_each(std::execution::par_unseq, itToId.begin(), itToId.end(), innerLoop);
			}
			else
				for (int y = 1; y < colorBuffer.getRes().y; ++y)
					innerLoop(y);

			if constexpr (ColorBufferEditor::IsDoubleBuffering())
				editor->swapBuffers();
		}

		void plasma(auto& colorBuffer)
		{
			const float newColorFactor = 0.25001f;
			const glm::vec3 initRgbMin = glm::vec3(-100);
			const glm::vec3 initRgbMax = glm::vec3(100);
			const auto originalBottom = colorBuffer.getBottom();

			colorBuffer.setBottom(colorBuffer.Bottom::Down);
			flames(colorBuffer, newColorFactor, initRgbMin, initRgbMax);
			colorBuffer.setBottom(colorBuffer.Bottom::Right);
			flames(colorBuffer, newColorFactor, initRgbMin, initRgbMax);
			colorBuffer.setBottom(colorBuffer.Bottom::Up);
			flames(colorBuffer, newColorFactor, initRgbMin, initRgbMax);
			colorBuffer.setBottom(colorBuffer.Bottom::Left);
			flames(colorBuffer, newColorFactor, initRgbMin, initRgbMax);

			colorBuffer.setBottom(originalBottom);
		}

		void colorChanneling(auto& colorBuffer)
		{
			const float newColorFactor = 0.25f;
			const auto originalBottom = colorBuffer.getBottom();

			colorBuffer.setBottom(colorBuffer.Bottom::Left);
			flames(colorBuffer, newColorFactor);
			colorBuffer.setBottom(colorBuffer.Bottom::Right);
			flames(colorBuffer, newColorFactor);

			colorBuffer.setBottom(originalBottom);
		}

		ComponentId textureId;
		std::unique_ptr<ColorBufferEditor> editor;
		std::string texturePath = "textures/rose.png";
		int effect = 0;
		glm::vec2 cursorPos = { 0.0f, 0.0f };
		glm::vec3 cursorColor = { 0.8f, 0.2f, 0.8f };
		float cursorSize = 0.05f;
	};

	Paint::Paint():
		impl(std::make_unique<Impl>())
	{
		impl->setup();
	}

	void Paint::step()
	{
		impl->step();
	}
}
