#include "camera.hpp"

#include <components/camera2D.hpp>
#include <components/camera3D.hpp>
#include <components/mvp.hpp>
#include <components/screenInfo.hpp>
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
		auto& camera = Globals::Components().camera2D();
		const float targetProjectionHSize = camera.targetProjectionHSizeF();
		camera.details.position = camera.targetPositionF();
		camera.details.prevPosition = camera.details.position;
		camera.details.prevProjectionHSize = targetProjectionHSize;

		postInit3D();
	}

	void Camera::step(bool paused) const
	{
		const auto& screenInfo = Globals::Components().screenInfo();
		auto& camera = Globals::Components().camera2D();
		auto& mvp = Globals::Components().mvp2D();
		auto& physics = Globals::Components().physics();

		const float windowWidthRatio = screenInfo.windowSize.x > screenInfo.windowSize.y
			? screenInfo.getAspectRatio()
			: 1.0f;
		const float windowHeightRatio = screenInfo.windowSize.x < screenInfo.windowSize.y
			? 1.0f / screenInfo.getAspectRatio()
			: 1.0f;

		if (!paused)
		{
			const float targetProjectionHSize = camera.targetProjectionHSizeF();
			const glm::vec2 targetPosition = camera.targetPositionF();

			// TODO: Steps dependent on frame time.
			camera.details.projectionHSize = camera.details.prevProjectionHSize + (targetProjectionHSize - camera.details.prevProjectionHSize)
				* std::clamp(camera.projectionTransitionFactor * physics.frameDuration, 0.0f, 1.0f);
			camera.details.position = camera.details.prevPosition + (targetPosition - camera.details.prevPosition)
				* std::clamp(camera.positionTransitionFactor * physics.frameDuration, 0.0f, 1.0f);

			camera.details.prevProjectionHSize = camera.details.projectionHSize;
			camera.details.prevPosition = camera.details.position;

			mvp.view = glm::translate(glm::mat4(1.0f), glm::vec3(-camera.details.position, 0.0f));
		}
		
		mvp.projection = glm::ortho(-camera.details.projectionHSize * windowWidthRatio, camera.details.projectionHSize * windowWidthRatio,
			-camera.details.projectionHSize * windowHeightRatio, camera.details.projectionHSize * windowHeightRatio);

		step3D(paused);
	}

	void Camera::postInit3D() const
	{
	}

	void Camera::step3D(bool paused) const
	{
		const auto& screenInfo = Globals::Components().screenInfo();
		const auto& camera = Globals::Components().camera3D();
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
