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

	void Camera::initializationFinalize() const
	{
		auto& camera = Globals::Components().camera();
		const float targetProjectionHSize = camera.targetProjectionHSizeF();
		const glm::vec2 targetPosition = camera.targetPositionF();

		camera.prevProjectionHSize = targetProjectionHSize;
		camera.prevPosition = targetPosition;
	}

	void Camera::step() const
	{
		const auto& screenInfo = Globals::Components().screenInfo();
		auto& camera = Globals::Components().camera();
		auto& mvp = Globals::Components().mvp();

		const float windowWidthRatio = screenInfo.windowSize.x > screenInfo.windowSize.y
			? (float)screenInfo.windowSize.x / screenInfo.windowSize.y
			: 1.0f;
		const float windowHeightRatio = screenInfo.windowSize.x < screenInfo.windowSize.y
			? (float)screenInfo.windowSize.y / screenInfo.windowSize.x
			: 1.0f;
		const float targetProjectionHSize = camera.targetProjectionHSizeF();
		const glm::vec2 targetPosition = camera.targetPositionF();

		const float projectionHSize = camera.prevProjectionHSize + (targetProjectionHSize - camera.prevProjectionHSize)
			* std::clamp(camera.projectionTransitionFactor, 0.0f, 1.0f);
		const glm::vec2 position = camera.prevPosition + (targetPosition - camera.prevPosition)
			* std::clamp(camera.positionTransitionFactor, 0.0f, 1.0f);

		camera.prevProjectionHSize = projectionHSize;
		camera.prevPosition = position;

		mvp.view = glm::translate(glm::mat4(1.0f), glm::vec3(-position, 0.0f));
		mvp.projection = glm::ortho(-projectionHSize * windowWidthRatio, projectionHSize * windowWidthRatio,
			-projectionHSize * windowHeightRatio, projectionHSize * windowHeightRatio);
	}
}
