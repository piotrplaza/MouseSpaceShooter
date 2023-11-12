#include "basic3D.hpp"

#include <components/decoration.hpp>
#include <components/physics.hpp>

#include <ogl/shaders/basicPhong.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace Levels
{
	class Basic3D::Impl
	{
	public:
		void shadersSetup() const
		{
			Globals::Shaders().basicPhong().numOfLights(1);
			Globals::Shaders().basicPhong().lightsPos(0, {0.0f, 0.0f, 0.0f});
			Globals::Shaders().basicPhong().lightsCol(0, { 1.0f, 1.0f, 1.0f });
		}

		void createDecorations() const
		{
			const auto& physics = Globals::Components().physics();
			auto& box = Globals::Components().staticDecorations().emplace();

			box.vertices = {
				{-1.0f, 0.0f, 0.0f},
				{0.0f, -1.0f, 0.0f},
				{0.0f, 1.0f, 0.0f},
				{1.0f, 0.0f, 0.0f}
			};

			box.colors = {
				{1.0f, 0.0f, 0.0f, 1.0f},
				{0.0f, 1.0f, 0.0f, 1.0f},
				{0.0f, 0.0f, 1.0f, 1.0f},
				{1.0f, 1.0f, 0.0f, 1.0f}
			};

			box.indices = {
				0, 1, 2, 1, 3, 2
			};

			box.params3D = RenderableDef::Params3D{}.normals({
				{0.0f, 0.0f, 1.0f},
				{0.0f, 0.0f, 1.0f},
				{0.0f, 0.0f, 1.0f},
				{0.0f, 0.0f, 1.0f}});

			box.modelMatrixF = [&]() { return glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, -2.0f }), physics.simulationDuration * 1.5f, { 1.0f, 1.0f, 1.0f }); };
		}
	};

	Basic3D::Basic3D():
		impl(std::make_unique<Impl>())
	{
		impl->shadersSetup();
		impl->createDecorations();
	}
}
