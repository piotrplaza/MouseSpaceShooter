#include "camera.hpp"

#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

#include <globals.hpp>

#include <components/camera.hpp>
#include <components/mvp.hpp>
#include <components/screenInfo.hpp>

namespace Systems
{
	Camera::Camera() = default;

	void Camera::step()
	{
		using namespace Globals::Components;

		const float windowWidthRatio = screenInfo.windowSize.x > screenInfo.windowSize.y
			? (float)screenInfo.windowSize.x / screenInfo.windowSize.y
			: 1.0f;
		const float windowHeightRatio = screenInfo.windowSize.x < screenInfo.windowSize.y
			? (float)screenInfo.windowSize.y / screenInfo.windowSize.x
			: 1.0f;
		const float targetProjectionHSize = camera.projectionHSizeF();
		const glm::vec2 targetMainActorPosition = camera.mainActorPositionF();

		if (firstStep)
		{
			prevProjectionHSize = targetProjectionHSize;
			prevMainActorPosition = targetMainActorPosition;
			firstStep = false;
		}

		const float projectionHSize = prevProjectionHSize + (targetProjectionHSize - prevProjectionHSize)
			* std::clamp(camera.projectionTransitionFactor, 0.0f, 1.0f);
		const glm::vec2 mainActorPosition = prevMainActorPosition + (targetMainActorPosition - prevMainActorPosition)
			* std::clamp(camera.positionTransitionFactor, 0.0f, 1.0f);

		prevProjectionHSize = projectionHSize;
		prevMainActorPosition = mainActorPosition;

		mvp.view = glm::translate(glm::mat4(1.0f), glm::vec3(-mainActorPosition, 0.0f));
		mvp.projection = glm::ortho(-projectionHSize * windowWidthRatio, projectionHSize * windowWidthRatio,
			-projectionHSize * windowHeightRatio, projectionHSize * windowHeightRatio);
	}
}
