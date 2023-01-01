#include "camera.hpp"

#include <components/camera.hpp>
#include <components/mvp.hpp>
#include <components/screenInfo.hpp>

#include <globals/components.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

namespace Systems
{
	Camera::Camera() = default;

	void Camera::postInit() const
	{
		auto& camera = Globals::Components().camera();
		const float targetProjectionHSize = camera.targetProjectionHSizeF();
		camera.details.position = camera.targetPositionF();
		camera.details.prevPosition = camera.details.position;
		camera.details.prevProjectionHSize = targetProjectionHSize;
	}

	void Camera::step() const
	{
		const auto& screenInfo = Globals::Components().screenInfo();
		auto& camera = Globals::Components().camera();
		auto& mvp = Globals::Components().mvp();

		const float windowWidthRatio = screenInfo.windowSize.x > screenInfo.windowSize.y
			? screenInfo.getAspectRatio()
			: 1.0f;
		const float windowHeightRatio = screenInfo.windowSize.x < screenInfo.windowSize.y
			? 1.0f / screenInfo.getAspectRatio()
			: 1.0f;
		const float targetProjectionHSize = camera.targetProjectionHSizeF();
		const glm::vec2 targetPosition = camera.targetPositionF();

		camera.details.projectionHSize = camera.details.prevProjectionHSize + (targetProjectionHSize - camera.details.prevProjectionHSize)
			* std::clamp(camera.projectionTransitionFactor, 0.0f, 1.0f);
		camera.details.position = camera.details.prevPosition + (targetPosition - camera.details.prevPosition)
			* std::clamp(camera.positionTransitionFactor, 0.0f, 1.0f);

		camera.details.prevProjectionHSize = camera.details.projectionHSize;
		camera.details.prevPosition = camera.details.position;

		mvp.view = glm::translate(glm::mat4(1.0f), glm::vec3(-camera.details.position, 0.0f));
		mvp.projection = glm::ortho(-camera.details.projectionHSize * windowWidthRatio, camera.details.projectionHSize * windowWidthRatio,
			-camera.details.projectionHSize * windowHeightRatio, camera.details.projectionHSize * windowHeightRatio);
	}
}
