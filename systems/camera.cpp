#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <globals.hpp>

#include <components/camera.hpp>
#include <components/mvp.hpp>
#include <components/screenInfo.hpp>

namespace Systems
{
	Camera::Camera()
	{
	}

	void Camera::step() const
	{
		using namespace Globals::Components;

		const float windowRatio = (float)screenInfo.windowSize.x / screenInfo.windowSize.y;
		const float hProjectionSize = camera.verticalProjectionHSizeF();
		const glm::vec2 mainActorPosition = camera.mainActorPositionF();

		mvp.view = glm::translate(glm::mat4(1.0f), glm::vec3(-mainActorPosition, 0.0f));
		mvp.projection = glm::ortho(-hProjectionSize * windowRatio, hProjectionSize * windowRatio, -hProjectionSize, hProjectionSize);
	}
}
