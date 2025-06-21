#include "decals.hpp"

#include <components/graphicsSettings.hpp>
#include <components/texture.hpp>
#include <components/decoration.hpp>
#include <components/camera2d.hpp>
#include <components/keyboard.hpp>
#include <components/mouse.hpp>
#include <components/physics.hpp>
#include <globals/components.hpp>

#include <systems/textures.hpp>
#include <globals/systems.hpp>

#include <tools/Shapes2D.hpp>
#include <tools/colorBufferEditor.hpp>

#include <execution>

namespace
{
	constexpr bool parallelProcessing = true;
	constexpr char mainTexturePath[] = "textures/play field.jpg";
	constexpr char decalTexturePath[] = "textures/skull rot.png";
}

namespace Levels
{
	class Decals::Impl
	{
	public:
		Decals::Impl():
			decalTextureSubData(TextureSubData::Params{}.imagesData({ { TextureData(TextureFile(decalTexturePath, 4)), {} } })),
			decalTextureData(TextureFile(decalTexturePath, 4))
		{
			Globals::Components().graphicsSettings().backgroundColorF = glm::vec4{ 0.0f, 0.1f, 0.1f, 1.0f };
			Globals::Components().camera2D().targetPositionAndProjectionHSizeF = glm::vec3(0.0f, 0.0f, 0.5f);
			auto& texture = Globals::Components().textures().emplace(TextureData(TextureFile(mainTexturePath, 3)));
			//texture.magFilter = GL_NEAREST;
			texture.wrapMode = GL_CLAMP_TO_EDGE;
			//texture.sourceFragmentCornerAndSizeF = [marigin = 5](glm::ivec2 origSize) { return std::make_pair(glm::ivec2(marigin, marigin), origSize - glm::ivec2(marigin * 2)); };
			//texture.preserveAspectRatio = true;
			//texture.translate = { 0.0f, -0.05f };
			//texture.scale = { 1.12f, 1.12f };
			mainTextureId = texture.getComponentId();
		}

		void postSetup()
		{
			auto& texture = Globals::Components().textures()[mainTextureId];
			auto& staticDecorations = Globals::Components().staticDecorations();
			staticDecorations.emplace(Tools::Shapes2D::CreatePositionsOfRectangle({ 0.0f, 0.0f }, { 0.5f * texture.loaded.getAspectRatio(), 0.5f }), CM::Texture(mainTextureId, false), Tools::Shapes2D::CreateTexCoordOfRectangle());
			auto& cursor = staticDecorations.emplace(Tools::Shapes2D::CreatePositionsOfDisc({ 0.0f, 0.0f }, 1.0f, 100));
			cursor.colorF = [&]() { return glm::vec4(cursorColor, 1.0f); };
			cursor.modelMatrixF = [&]() {
				return glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(cursorPos, 0.0f)), glm::vec3(glm::vec2(cursorHSize) * 0.1f, 1.0f));
			};
			cursor.renderLayer = RenderLayer::NearForeground;
		}

		void step()
		{
			const auto& keyboard = Globals::Components().keyboard();
			const auto& mouse = Globals::Components().mouse();
			auto& texture = Globals::Components().textures()[mainTextureId];
			auto& loadedTextureData = std::get<TextureData>(texture.source).loaded;
			auto& buffer = std::get<std::vector<glm::vec3>>(loadedTextureData.data);
			const auto& textureSize = loadedTextureData.size;

			cursorPos += mouse.getCartesianDelta() * 0.0005f;
			cursorPos.x = std::clamp(cursorPos.x, -0.5f * texture.loaded.getAspectRatio(), 0.5f * texture.loaded.getAspectRatio());
			cursorPos.y = std::clamp(cursorPos.y, -0.5f, 0.5f);

			if (!editor)
			{
				editor = std::make_unique<ColorBufferEditor>(buffer, textureSize);
				//editor->setBorder(glm::vec3(0.0f));
			}

			if (!decalTextureData.file.path.empty())
				Globals::Systems().textures().textureDataFromFile(decalTextureData);

#define TEST 6
#if TEST != 7
			switch (effect)
			{
			case 0: blur(*editor); break;
			case 1: plasma(*editor); break;
			case 2: colorChanneling(*editor); break;
			case 3: flames(*editor); break;
			case 4: none(*editor); break;
			}
#endif

			if (mouse.pressing.lmb)
			{
				const glm::vec2 normalizedCursorPos = (cursorPos - glm::vec2(-0.5f * texture.loaded.getAspectRatio(), -0.5f)) / glm::vec2(texture.loaded.getAspectRatio(), 1.0f);
				const glm::ivec2 cursorPosInTexture = normalizedCursorPos * glm::vec2(textureSize) + 0.5f;
#if TEST == 0
				editor->putRectangle(cursorPosInTexture, { cursorHSize * textureSize.y, cursorHSize * textureSize.y }, cursorColor);
#elif TEST == 1
				editor->putCircle(cursorPosInTexture, int(cursorHSize * textureSize.y), cursorColor);
#elif TEST == 2
				editor->putEllipse(cursorPosInTexture, { cursorHSize * textureSize.y * 2, cursorHSize * textureSize.y }, cursorColor);
#elif TEST == 3
				{
					const auto size = glm::ivec2(cursorHSize * textureSize.y, cursorHSize * textureSize.y) * 2;
					const glm::ivec2 offset(cursorPosInTexture - size / 2);
					std::vector<glm::vec3> data; (size.x * size.y, glm::vec3(Tools::RandomFloat(0.0f, 1.0f), Tools::RandomFloat(0.0f, 1.0f), Tools::RandomFloat(0.0f, 1.0f)));
					data.reserve(size.x * size.y);
					for (int y = 0; y < size.y; ++y)
						for (int x = 0; x < size.x; ++x)
							data.push_back(glm::vec3(Tools::RandomFloat(0.0f, 1.0f), Tools::RandomFloat(0.0f, 1.0f), Tools::RandomFloat(0.0f, 1.0f)));
					texture.subImagesF = [textureSubData = TextureSubData(TextureSubData::Params{}.imagesData({ { TextureData(std::move(data), size), offset } }).exclusiveLoad(false))]() mutable -> auto& {
						return textureSubData;
					};
				}
#elif TEST == 4
				texture.subImagesF = [textureSubData = TextureSubData(TextureSubData::Params{}.imagesData({ { TextureData(TextureFile(decalTexturePath, 3)), {} } })
					.deferredOffsetPosF([=](const auto& size, auto, auto) { return cursorPosInTexture - size / 2; }))]() mutable -> auto& {
					return textureSubData;
				};
#elif TEST == 5
				{
					decalTextureSubData.deferredOffsetPosF = [=](const auto& size, auto, auto) { return cursorPosInTexture - size / 2; };
					texture.subImagesF = [&]() mutable -> auto& { return decalTextureSubData; };
				}
#elif TEST == 6
				editor->updateSubImage(decalTextureData.getRawData(), decalTextureData.loaded.size, cursorPosInTexture - decalTextureData.loaded.size / 2, decalTextureData.getNumOfChannels(), 1.0f);
#elif TEST == 7
				{
					texture.subImagesF = [&, cursorPosInTexture, operationalBuffer = std::vector<float>()]() mutable {
						const glm::ivec2 subImgSize = decalTextureData.loaded.size;
						const glm::ivec2 subImgOffset = cursorPosInTexture - subImgSize / 2;
						editor->updateSubImage(decalTextureData.getRawData(), subImgSize, subImgOffset, decalTextureData.getNumOfChannels(), 1.0f);
						const auto subImage = editor->getSubImage(subImgOffset, subImgSize, operationalBuffer);
						auto textureData = TextureData(subImage.data, subImage.size, editor->getNumOfChannels());
						auto textureSubData = TextureSubData(TextureSubData::Params{}.imagesData({ { std::move(textureData), subImage.offset } }).exclusiveLoad(true));
						return textureSubData;
					};
				}
#endif
			}

			for (int i = 0; i < 5; ++i)
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
				auto& texture = Globals::Components().textures()[mainTextureId];
				texture.source = TextureData(TextureFile(mainTexturePath, 3));
				editor.reset();
			}
			else if constexpr (ColorBufferEditor::IsDoubleBuffering())
				editor->swapBuffers();

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
					const glm::ivec2 d = Tools::StableRandom::Std3Random::HashRange({ range.x, range.x }, { range.y, range.y }, glm::ivec3(x, y, physics.frameCount));
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

		void none(auto& colorBuffer)
		{
			if constexpr (ColorBufferEditor::IsDoubleBuffering())
			{
				Tools::ItToId itToId(colorBuffer.getRes().y);
				std::for_each(std::execution::par_unseq, itToId.begin(), itToId.end(), [&](const auto y) {
					for (int x = 0; x < colorBuffer.getRes().x; ++x)
						colorBuffer.putColor({ x, y }, colorBuffer.getColor({ x, y }));
				});
			}
		}

		ComponentId mainTextureId{};
		TextureSubData decalTextureSubData;
		TextureData decalTextureData;
		std::unique_ptr<ColorBufferEditor> editor;
		int effect = 0;
		glm::vec2 cursorPos = { 0.0f, 0.0f };
		glm::vec3 cursorColor = { 0.8f, 0.2f, 0.8f };
		float cursorHSize = 0.05f;
	};

	Decals::Decals():
		impl(std::make_unique<Impl>())
	{
	}

	void Decals::postSetup()
	{
		impl->postSetup();
	}

	void Decals::step()
	{
		impl->step();
	}
}
