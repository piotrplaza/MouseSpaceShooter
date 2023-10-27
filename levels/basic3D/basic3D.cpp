#include "basic3D.hpp"

#include <components/decoration.hpp>
#include <components/physics.hpp>

#include <globals/components.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace Levels
{
	class Basic3D::Impl
	{
	public:
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
			box.normals = {
				{0.0f, 0.0f, 1.0f},
				{0.0f, 0.0f, 1.0f},
				{0.0f, 0.0f, 1.0f},
				{0.0f, 0.0f, 1.0f}
			};
			box.colors = {
				{1.0f, 0.0f, 0.0f, 1.0f},
				{0.0f, 1.0f, 0.0f, 1.0f},
				{0.0f, 0.0f, 1.0f, 1.0f},
				{0.0f, 1.0f, 1.0f, 1.0f}
			};
			box.indices = {
				0, 1, 2, 1, 3, 2
			};
			box.modelMatrixF = [&]() { return glm::rotate(glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, -2.0f }), physics.simulationDuration * 1.5f, { 1.0f, 1.0f, 1.0f }); };
		}
	};

	Basic3D::Basic3D():
		impl(std::make_unique<Impl>())
	{
		impl->createDecorations();
	}
}
