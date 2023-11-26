#include "basic3D.hpp"

#include <components/decoration.hpp>
#include <components/physics.hpp>

#include <ogl/shaders/basicPhong.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

#include <tools/shapes3D.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace Levels
{
	class Basic3D::Impl
	{
	public:
		void shadersSetup() const
		{
			auto& basicPhong = Globals::Shaders().basicPhong();
			basicPhong.numOfLights(1);
			basicPhong.lightsPos(0, {0.0f, 0.0f, 0.0f});
			basicPhong.lightsCol(0, { 1.0f, 1.0f, 1.0f });
			basicPhong.lightsAttenuation(0, 1.0f);
			basicPhong.ambient(0.1f);
			basicPhong.diffuse(0.8f);
			basicPhong.specular(3.0f);
			basicPhong.specularFocus(8.0f);
		}

		void createDecorations() const
		{
			const auto& physics = Globals::Components().physics();
			auto& box = Globals::Components().staticDecorations().emplace();
			/*Shapes3D::SetRectangle(box, { 0.8f, 0.8f }, { { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
				glm::rotate(glm::mat4(1.0f), glm::quarter_pi<float>(), {0.0f, 0.0f, 1.0f}));*/
			Shapes3D::SetCuboid(box, { 0.5f, 0.5f, 0.5f }, { { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } });
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
