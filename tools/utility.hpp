#pragma once

#include <glm/vec2.hpp>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Tools
{
	void RedirectIOToConsole(glm::ivec2 windowPos = {10, 10}, int maxConsoleLines = 500);

	glm::vec2 GetNormalizedMousePosition();
	void SetMousePos(glm::ivec2 mousePos);
	glm::ivec2 GetMousePos();
	void SetMouseCursorVisibility(bool visibility);

	void RandomInit();
	float Random(float min, float max);
	unsigned StableRandom(unsigned seed);
	float StableRandom(float min, float max, unsigned seed);

	template <class T>
	inline void HashCombine(std::size_t& seed, const T& v)
	{
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	inline glm::mat4 TextureTransform(glm::vec2 translate, float angle = 0.0f, glm::vec2 scale = { 1.0f, 1.0f })
	{
		return
			glm::translate(
				glm::rotate(
					glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / scale.x, 1.0f / scale.y, 1.0f)),
					angle, { 0.0f, 0.0f, -1.0f }),
				glm::vec3(-translate, 0.0f));
	}

	template <typename TextureComponent>
	inline glm::mat4 TextureTransform(const TextureComponent& textureComponent, bool preserveTextureRatio)
	{
		return
			glm::translate(
				glm::scale(glm::mat4(1.0f), glm::vec3(
					(preserveTextureRatio ? (float)textureComponent.loaded.size.y / textureComponent.loaded.size.x : 1.0f)
					* 1.0f / textureComponent.scale.x, 1.0f / textureComponent.scale.y, 1.0f)),
				glm::vec3(-textureComponent.translate, 0.0f));
	}
}
