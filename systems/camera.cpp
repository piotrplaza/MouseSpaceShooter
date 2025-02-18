#include "camera.hpp"

#include <components/camera2D.hpp>
#include <components/camera3D.hpp>
#include <components/mvp.hpp>
#include <components/systemInfo.hpp>
#include <components/physics.hpp>

#include <globals/components.hpp>

#include <tools/utility.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <algorithm>
#include <variant>

namespace Systems
{
	Camera::Camera() = default;

	void Camera::postInit() const
	{
		const auto& screenInfo = Globals::Components().systemInfo().screen;

		const float windowWidthRatio = screenInfo.windowRes.x > screenInfo.windowRes.y
			? screenInfo.getAspectRatio()
			: 1.0f;
		const float windowHeightRatio = screenInfo.windowRes.x < screenInfo.windowRes.y
			? 1.0f / screenInfo.getAspectRatio()
			: 1.0f;

		auto& camera = Globals::Components().camera2D();
		const glm::vec3 targetPositionAndProjectionHSize = camera.targetPositionAndProjectionHSizeF();
		camera.details.position = glm::vec2(targetPositionAndProjectionHSize);
		camera.details.prevPosition = camera.details.prevPosition = camera.details.position;
		camera.details.projectionHSize = camera.details.prevProjectionHSize = targetPositionAndProjectionHSize.z;

		camera.details.completeProjectionHSize = glm::vec2(camera.details.projectionHSize * windowWidthRatio, camera.details.projectionHSize * windowHeightRatio);
		camera.details.prevCompleteProjectionHSize = glm::vec2(camera.details.prevProjectionHSize * windowWidthRatio, camera.details.prevProjectionHSize * windowHeightRatio);
	}

	void Camera::step() const
	{
		step2D(Globals::Components().physics().paused);
		step3D(Globals::Components().physics().paused);
	}

	void Camera::step2D(bool paused) const
	{
		auto& camera = Globals::Components().camera2D();

		if (!camera.isEnabled())
			return;

		const auto& screenInfo = Globals::Components().systemInfo().screen;
		auto& mvp = Globals::Components().mvp2D();
		auto& physics = Globals::Components().physics();

		const float windowWidthRatio = screenInfo.windowRes.x > screenInfo.windowRes.y
			? screenInfo.getAspectRatio()
			: 1.0f;
		const float windowHeightRatio = screenInfo.windowRes.x < screenInfo.windowRes.y
			? 1.0f / screenInfo.getAspectRatio()
			: 1.0f;

		if (!paused)
		{
			const glm::vec3 targetPositionAndProjectionHSize = camera.targetPositionAndProjectionHSizeF();

			camera.details.projectionHSize = camera.details.prevProjectionHSize + (targetPositionAndProjectionHSize.z - camera.details.prevProjectionHSize)
				* std::clamp(camera.projectionTransitionFactor * physics.frameDuration, 0.0f, 1.0f);
			camera.details.position = camera.details.prevPosition + (glm::vec2(targetPositionAndProjectionHSize) - camera.details.prevPosition)
				* std::clamp(camera.positionTransitionFactor * physics.frameDuration, 0.0f, 1.0f);

			camera.details.prevProjectionHSize = camera.details.projectionHSize;
			camera.details.prevPosition = camera.details.position;

			camera.details.completeProjectionHSize = glm::vec2(camera.details.projectionHSize * windowWidthRatio, camera.details.projectionHSize * windowHeightRatio);
			camera.details.prevCompleteProjectionHSize = glm::vec2(camera.details.prevProjectionHSize * windowWidthRatio, camera.details.prevProjectionHSize * windowHeightRatio);

			mvp.view = glm::translate(glm::mat4(1.0f), glm::vec3(-camera.details.position, 0.0f));
		}

		mvp.projection = glm::ortho(-camera.details.projectionHSize * windowWidthRatio, camera.details.projectionHSize * windowWidthRatio,
			-camera.details.projectionHSize * windowHeightRatio, camera.details.projectionHSize * windowHeightRatio);
	}

	void Camera::step3D(bool paused) const
	{
		const auto& camera = Globals::Components().camera3D();

		if (!camera.isEnabled())
			return;

		const auto& screenInfo = Globals::Components().systemInfo().screen;
		auto& mvp = Globals::Components().mvp3D();

		if (!paused)
		{
			std::visit(Tools::Overloads{
				[&](const Components::Camera3D::EulerRotation& rotation) {
					mvp.view = glm::inverse(glm::eulerAngleYXZ(-rotation.y, rotation.x, rotation.z)) * glm::translate(glm::mat4(1.0f), -camera.position);
				},
				[&](const Components::Camera3D::LookAtRotation& rotation) {
					mvp.view = glm::lookAt(camera.position, rotation.target, rotation.up);
				}
				}, camera.rotation);
		}

		mvp.projection = glm::perspective(camera.fov, screenInfo.getAspectRatio(), camera.nearPlane, camera.farPlane);
	}
}
