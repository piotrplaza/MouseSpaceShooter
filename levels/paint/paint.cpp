#include "paint.hpp"

#include <components/graphicsSettings.hpp>
#include <components/texture.hpp>
#include <components/decoration.hpp>
#include <components/camera2d.hpp>
#include <components/keyboard.hpp>
#include <components/mouse.hpp>
#include <components/physics.hpp>

#include <globals/components.hpp>

#include <tools/Shapes2D.hpp>
#include <tools/colorBufferEditor.hpp>
#include <tools/utility.hpp>

#include <execution>

namespace
{
	constexpr bool parallelProcessing = true;
	constexpr char texturePath[] = "textures/rose.png";
}

namespace Levels
{
	class Paint::Impl
	{
	public:
		void setup()
		{
			Globals::Components().graphicsSettings().backgroundColorF = glm::vec4{ 0.0f, 0.1f, 0.1f, 1.0f };
			Globals::Components().camera2D().targetPositionAndProjectionHSizeF = glm::vec3(0.0f, 0.0f, 0.5f);
			auto& texture = Globals::Components().textures().emplace(TextureData(TextureFile(texturePath, 3)));
			//texture.magFilter = GL_NEAREST;
			texture.wrapMode = GL_CLAMP_TO_EDGE;
			//texture.sourceFragmentCornerAndSizeF = [marigin = 5](glm::ivec2 origSize) { return std::make_pair(glm::ivec2(marigin, marigin), origSize - glm::ivec2(marigin * 2)); };
			textureId = texture.getComponentId();

			auto& staticDecorations = Globals::Components().staticDecorations();
			staticDecorations.emplace(Tools::Shapes2D::CreateVerticesOfRectangle(), CM::Texture(textureId, false), Tools::Shapes2D::CreateTexCoordOfRectangle());
			auto& cursor = staticDecorations.emplace(Tools::Shapes2D::CreateVerticesOfCircle({ 0.0f, 0.0f }, 1.0f, 100));
			cursor.colorF = [&]() { return glm::vec4(cursorColor, 1.0f); };
			cursor.modelMatrixF = [&]() {
				cursorPos += Globals::Components().mouse().getCartesianDelta() * 0.0005f;
				cursorPos.x = std::clamp(cursorPos.x, -0.5f, 0.5f);
				cursorPos.y = std::clamp(cursorPos.y, -0.5f, 0.5f);
				return glm::translate(glm::mat4(1.0f), glm::vec3(cursorPos, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(cursorHSize, cursorHSize, 1.0f)
				);
			};
			cursor.renderLayer = RenderLayer::NearForeground;
		}

		void step()
		{
			const auto& keyboard = Globals::Components().keyboard();
			const auto& mouse = Globals::Components().mouse();
			auto& texture = Globals::Components().textures()[textureId];
			auto& loadedTextureData = std::get<TextureData>(texture.source).loaded;
			auto& buffer = std::get<std::vector<glm::vec3>>(loadedTextureData.data);
			const auto& textureSize = loadedTextureData.size;

			if (!editor)
				editor = std::make_unique<ColorBufferEditor>(buffer, textureSize);	

			switch (effect)
			{
			case 0: blur(*editor); break;
			case 1: plasma(*editor); break;
			case 2: colorChanneling(*editor); break;
			case 3: flames(*editor); break;
			}

			if (mouse.pressing.lmb)
			{
				if constexpr (ColorBufferEditor::IsDoubleBuffering())
					editor->swapBuffers(false);

				const auto cursorPosInTexture = glm::ivec2((cursorPos + glm::vec2(0.5f)) * glm::vec2(textureSize - 1));
				//editor->putRectangle(cursorPosInTexture, { cursorHSize * textureSize.x, cursorHSize * textureSize.y }, cursorColor);
				//editor->putCircle(cursorPosInTexture, int(cursorHSize * textureSize.y), cursorColor);
				editor->putEllipse(cursorPosInTexture, { cursorHSize * textureSize.x, cursorHSize * textureSize.y }, cursorColor);

				if constexpr (ColorBufferEditor::IsDoubleBuffering())
					editor->swapBuffers(false);
			}

			for (int i = 0; i < 4; ++i)
				if (keyboard.pressed[(int)'1' + i])
					effect = i;

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
				cursorHSize += mouse.pressed.wheel * sizeStep * (cursorHSize + 0.1f);
				cursorHSize = std::clamp(cursorHSize, 0.0f, 1.0f);
			}

			cursorColor.r = std::clamp(cursorColor.r, 0.0f, 1.0f);
			cursorColor.g = std::clamp(cursorColor.g, 0.0f, 1.0f);
			cursorColor.b = std::clamp(cursorColor.b, 0.0f, 1.0f);

			if (keyboard.pressing[0x20/*VK_SPACE*/])
			{
				auto& texture = Globals::Components().textures()[textureId];
				texture.source = TextureData(TextureFile(texturePath, 3));
				editor.reset();
			}

			texture.state = ComponentState::Changed;
		}

	private:
		using ColorBufferEditor = Tools::ColorBufferEditor<glm::vec3, parallelProcessing && 1>;

		void blur(auto& colorBuffer, float centerColorFactor = 1.0f / 9.0f, glm::ivec2 range = { 1, 5 })
		{
			const auto& physics = Globals::Components().physics();
			const float neighborsColorFactor = 1.0f - centerColorFactor;
			auto innerLoop = [&](const auto y) {
				for (int x = 0; x < colorBuffer.getRes().x; ++x)
				{
					const glm::ivec2 d = Tools::StableRandom::Std3Random::HashRange(glm::ivec2(range.x, range.x), glm::ivec2(range.y, range.y), glm::ivec3(x, y, physics.frameCount));
					const glm::vec3 newColor = colorBuffer.getColor({ x, y }) * centerColorFactor +
						(colorBuffer.getColor({ x - d.x, y - d.y }) + colorBuffer.getColor({ x, y - d.y }) + colorBuffer.getColor({ x + d.x, y - d.y }) + colorBuffer.getColor({ x - d.x, y }) + colorBuffer.getColor({ x + d.x, y }) +
							colorBuffer.getColor({ x - d.x, y + d.y }) + colorBuffer.getColor({ x, y + d.y }) + colorBuffer.getColor({ x + d.x, y + d.y })) / 8.0f * neighborsColorFactor;
					colorBuffer.putColor({ x, y }, newColor);
				}
				};

			if constexpr (ColorBufferEditor::IsDoubleBuffering())
			{
				Tools::ItToId itToId(colorBuffer.getRes().y);
				std::for_each(std::execution::par_unseq, itToId.begin(), itToId.end(), innerLoop);
			}
			else
				for (int y = 0; y < colorBuffer.getRes().y; ++y)
					innerLoop(y);

			if constexpr (ColorBufferEditor::IsDoubleBuffering())
				editor->swapBuffers(false);
		}

		void flames(auto& colorBuffer, float newColorFactor = 0.249f, glm::vec3 initRgbMin = glm::vec3(-200), glm::vec3 initRgbMax = glm::vec3(200))
		{
			for (int x = 0; x < colorBuffer.getRes().x; ++x)
				colorBuffer.putColor({ x, 0 }, { Tools::RandomFloat(initRgbMin.r, initRgbMax.r), Tools::RandomFloat(initRgbMin.g, initRgbMax.g), Tools::RandomFloat(initRgbMin.b, initRgbMax.b) });

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
				editor->swapBuffers(false);
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

		ComponentId textureId{};
		std::unique_ptr<ColorBufferEditor> editor;
		int effect = 0;
		glm::vec2 cursorPos = { 0.0f, 0.0f };
		glm::vec3 cursorColor = { 0.8f, 0.2f, 0.8f };
		float cursorHSize = 0.05f;
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
