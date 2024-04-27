#include "paint.hpp"

#include <components/graphicsSettings.hpp>
#include <components/texture.hpp>
#include <components/decoration.hpp>
#include <components/camera2d.hpp>

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
			auto& texture = Globals::Components().dynamicTextures().emplace(TextureData(TextureFile("textures/city.jpg", 3)));
			texture.magFilter = GL_NEAREST;
			texture.wrapMode = GL_CLAMP_TO_EDGE;
			texture.sourceFragmentCornerAndSizeF = [marigin = 5](glm::ivec2 origSize) { return std::make_pair(glm::ivec2(marigin, marigin), origSize - glm::ivec2(marigin * 2)); };
			textureId = texture.getComponentId();
			Globals::Components().staticDecorations().emplace(Shapes2D::CreateVerticesOfRectangle(), CM::DynamicTexture(textureId), Shapes2D::CreateTexCoordOfRectangle());
		}

		void step()
		{
			auto& texture = Globals::Components().dynamicTextures()[textureId];
			auto& loadedTextureData = std::get<TextureData>(texture.source).loaded;
			auto& buffer = std::get<std::vector<glm::vec3>>(loadedTextureData.data);
			auto& textureSize = loadedTextureData.size;

			if (!editor)
				editor = std::make_unique<ColorBufferEditor>(buffer, textureSize);

			plasma(*editor);

			texture.state = ComponentState::Changed;
		}

	private:
		using ColorBufferEditor = Tools::ColorBufferEditor<glm::vec3, true>;

		void flames(auto& colorBuffer, float newColorFactor = 0.249f, glm::vec3 initRgbMin = glm::vec3(-200), glm::vec3 initRgbMax = glm::vec3(200))
		{
			const float range = 200.0f;

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
