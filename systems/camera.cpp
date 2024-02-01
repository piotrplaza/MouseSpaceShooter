#include "camera.hpp"

#include <components/camera2D.hpp>
#include <components/camera3D.hpp>
#include <components/mvp.hpp>
#include <components/screenInfo.hpp>

#include <globals/components.hpp>

#include <tools/utility.hpp>

#include <glm/gtc/matrix_transform.hpp>

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

	void Camera::step() const
	{
		const auto& screenInfo = Globals::Components().screenInfo();
		auto& camera = Globals::Components().camera2D();
		auto& mvp = Globals::Components().mvp2D();

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

		step3D();
	}

	void Camera::postInit3D() const
	{
	}

	void Camera::step3D() const
	{
		const auto& screenInfo = Globals::Components().screenInfo();
		const auto& camera = Globals::Components().camera3D();
		auto& mvp = Globals::Components().mvp3D();

		mvp.projection = glm::perspective(camera.fov, screenInfo.getAspectRatio(), camera.nearPlane, camera.farPlane);

		std::visit(Tools::Overloads{
			[&](const glm::vec3& rotation) {
				mvp.view = glm::rotate(
					glm::rotate(
						glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)),
						rotation.x, glm::vec3(-1.0f, 0.0f, 0.0f)),
					rotation.y, glm::vec3(0.0f, 1.0f, 0.0f))
					* glm::translate(glm::mat4(1.0f), -camera.position);
			},
			[&](const std::pair<glm::vec3, glm::vec3>& rotation) {
				mvp.view = glm::lookAt(camera.position, rotation.first, rotation.second);
			}
		}, camera.rotation);
		
	}
}
