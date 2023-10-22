#include "basic3D.hpp"

#include <components/graphicsSettings.hpp>
#include <components/decoration.hpp>
#include <components/camera3D.hpp>

#include <globals/components.hpp>

namespace Levels
{
	class Basic3D::Impl
	{
	public:
		void setup()
		{
			Globals::Components().graphicsSettings().defaultColor = { 0.7f, 0.7f, 0.7f, 1.0f };
		}

		void setCamera() const
		{
			auto& camera = Globals::Components().camera3D();
		}

		void createDecorations() const
		{
			auto& decoration = Globals::Components().staticDecorations().emplace();
			decoration.vertices = {
				{-1.0f, 0.0f, 0.0f},
				{0.0f, -1.0f, 0.0f},
				{0.0f, 1.0f, 0.0f},
				{1.0f, 0.0f, 0.0f}
			};
			decoration.normals = {
				{0.0f, 0.0f, 1.0f},
				{0.0f, 0.0f, 1.0f},
				{0.0f, 0.0f, 1.0f},
				{0.0f, 0.0f, 1.0f}
			};
			decoration.colors = {
				{1.0f, 0.0f, 0.0f, 1.0f},
				{0.0f, 1.0f, 1.0f, 1.0f},
				{0.0f, 0.0f, 1.0f, 1.0f},
				{1.0f, 1.0f, 0.0f, 1.0f}
			};
			decoration.indices = {
				0, 1, 2, 1, 3, 2
			};
			decoration.colorF = []() { return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); };
		}

		void step()
		{
		}
	};

	Basic3D::Basic3D():
		impl(std::make_unique<Impl>())
	{
		impl->setCamera();
		impl->createDecorations();
		impl->setup();
	}

	Basic3D::~Basic3D() = default;

	void Basic3D::step()
	{
		impl->step();
	}
}
