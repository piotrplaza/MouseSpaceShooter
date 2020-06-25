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

		const float windowWidthRatio = screenInfo.windowSize.x > screenInfo.windowSize.y
			? (float)screenInfo.windowSize.x / screenInfo.windowSize.y
			: 1.0f;
		const float windowHeightRatio = screenInfo.windowSize.x < screenInfo.windowSize.y
			? (float)screenInfo.windowSize.y / screenInfo.windowSize.x
			: 1.0f;
		const float projectionHSize = camera.projectionHSizeF();
		const glm::vec2 mainActorPosition = camera.mainActorPositionF();

		mvp.view = glm::translate(glm::mat4(1.0f), glm::vec3(-mainActorPosition, 0.0f));
		mvp.projection = glm::ortho(-projectionHSize * windowWidthRatio, projectionHSize * windowWidthRatio,
			-projectionHSize * windowHeightRatio, projectionHSize * windowHeightRatio);
	}
}
