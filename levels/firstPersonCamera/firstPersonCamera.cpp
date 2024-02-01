#include "firstPersonCamera.hpp"

#include <components/decoration.hpp>
#include <components/physics.hpp>
#include <components/graphicsSettings.hpp>
#include <components/light3D.hpp>
#include <components/camera3D.hpp>
#include <components/mouse.hpp>
#include <components/keyboard.hpp>

#include <ogl/shaders/basicPhong.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <tools/shapes3D.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <Windows.h>

#include <algorithm>
#include <iostream>

namespace Levels
{
	class FirstPersonCamera::Impl
	{
	public:
		void shadersSetup() const
		{
			Globals::Components().graphicsSettings().clearColor = { 0.0f, 0.05f, 0.0f, 1.0f };
			Globals::Components().lights3D().emplace(glm::vec3(0.0f, 0.49f, 0.0f), glm::vec3(1.0f), 1.0f);
		}

		void createDecorations() const
		{
			auto& staticDecorations = Globals::Components().staticDecorations();

			auto& wallsAndInnerSphere = staticDecorations.emplace();
			Shapes3D::AddCuboid(wallsAndInnerSphere, { 0.5f, 0.5f, 0.5f }, { { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } }, glm::mat4(1.0f), true);
			Shapes3D::AddSphere(wallsAndInnerSphere, 0.1f, 50, 50, false);
			wallsAndInnerSphere.params3D->ambient(0.2f).specular(1.0f).specularFocus(32.0f);

			auto& lamp = staticDecorations.emplace();
			Shapes3D::AddSphere(lamp, 0.05f, 50, 50, false, nullptr, glm::translate(glm::mat4(1.0f), { 0.0f, 0.52f, 0.0f }));
			lamp.params3D->ambient(1.0f).diffuse(0.0f).specular(0.0f);
		}
	};

	FirstPersonCamera::FirstPersonCamera():
		impl(std::make_unique<Impl>())
	{
		impl->shadersSetup();
		impl->createDecorations();
	}

	void FirstPersonCamera::step()
	{
		const auto& physics = Globals::Components().physics();
		const auto& mouse = Globals::Components().mouse();
		const auto& keyboard = Globals::Components().keyboard();
		auto& camera = Globals::Components().camera3D();
		auto& rotation = std::get<glm::vec3>(Globals::Components().camera3D().rotation);

		const float moveSpeed = 0.5f;
		const float rotationSpeed = 1.5f;
		const float mouseSensitivity = 0.001f;

		auto positionStep = [&](const float radDirection) {
			camera.position.x += std::cos(rotation.y + radDirection) * moveSpeed * physics.frameDuration;
			camera.position.z += std::sin(rotation.y + radDirection) * moveSpeed * physics.frameDuration;
		};

		if (keyboard.pressing['W'])
			positionStep(-glm::half_pi<float>());
		if (keyboard.pressing['S'])
			positionStep(glm::half_pi<float>());
		if (keyboard.pressing['A'])
			positionStep(glm::pi<float>());
		if (keyboard.pressing['D'])
			positionStep(0.0f);

		if (keyboard.pressing[VK_SPACE])
			camera.position.y += moveSpeed * physics.frameDuration;
		if (keyboard.pressing[VK_SHIFT])
			camera.position.y -= moveSpeed * physics.frameDuration;

		if (keyboard.pressing['Q'])
			rotation.z = std::fmodf(rotation.z - rotationSpeed * physics.frameDuration, glm::two_pi<float>());
		if (keyboard.pressing['E'])
			rotation.z = std::fmodf(rotation.z + rotationSpeed * physics.frameDuration, glm::two_pi<float>());

		rotation.x = std::clamp(rotation.x + mouse.getCartesianDelta().y * mouseSensitivity, -glm::half_pi<float>(), glm::half_pi<float>());
		rotation.y = std::fmodf(rotation.y + mouse.getCartesianDelta().x * mouseSensitivity, glm::two_pi<float>());
	}
}
