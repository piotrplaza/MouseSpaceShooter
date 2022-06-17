#include "walls.hpp"

#include <ogl/oglProxy.hpp>
#include <ogl/buffersHelpers.hpp>
#include <ogl/renderingHelpers.hpp>

#include <ogl/shaders/basic.hpp>
#include <ogl/shaders/textured.hpp>

#include <components/physics.hpp>
#include <components/mvp.hpp>
#include <components/renderingSetup.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/texture.hpp>
#include <components/graphicsSettings.hpp>

#include <globals/shaders.hpp>

#include <Box2D/Box2D.h>

#include <GL/glew.h>

namespace Systems
{
	Walls::Walls() = default;

	void Walls::postInit()
	{
		initGraphics();
	}

	void Walls::step()
	{
		for (const auto& wall : Globals::Components().walls())
			if (wall.step)
				wall.step();

		for (const auto& [id, wall] : Globals::Components().dynamicWalls())
			if (wall.step)
				wall.step();

		for (auto& [id, grapple] : Globals::Components().grapples())
			if (grapple.step)
				grapple.step();

		updateDynamicBuffers();
	}

	void Walls::render() const
	{
		basicRender();
		texturedRender();
		customShadersRender();
	}

	void Walls::initGraphics()
	{
		updateStaticBuffers();
	}

	void Walls::updateStaticBuffers()
	{
		Tools::UpdateStaticBuffers(Globals::Components().walls(), staticBuffers.simpleWalls, staticBuffers.texturedWalls, staticBuffers.customShadersWalls);
	}

	void Walls::updateDynamicBuffers()
	{
		Tools::UpdateDynamicBuffers(Globals::Components().dynamicWalls(), dynamicBuffers.simpleWalls, dynamicBuffers.texturedWalls, dynamicBuffers.texturedWalls);
		Tools::UpdateDynamicBuffers(Globals::Components().grapples(), dynamicBuffers.simpleGrapples, dynamicBuffers.texturedGrapples, dynamicBuffers.texturedGrapples);
	}

	void Walls::customShadersRender() const
	{
		auto render = [&](const auto& buffers)
		{
			assert(buffers.customShadersProgram);
			glUseProgram_proxy(*buffers.customShadersProgram);

			std::function<void()> renderingTeardown;
			if (buffers.renderingSetup)
				renderingTeardown = Globals::Components().renderingSetups()[buffers.renderingSetup](*buffers.customShadersProgram);

			buffers.draw();

			if (renderingTeardown)
				renderingTeardown();
		};

		for (const auto& buffers : staticBuffers.customShadersWalls)
			render(buffers);

		for (const auto& buffers : staticBuffers.customShadersGrapples)
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers.customShadersWalls)
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers.customShadersGrapples)
			render(buffers);
	}

	void Walls::texturedRender() const
	{
		glUseProgram_proxy(Globals::Shaders().textured().getProgramId());
		Globals::Shaders().textured().vp(Globals::Components().mvp().getVP());

		auto render = [&](const auto& buffers)
		{
			Globals::Shaders().textured().color(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().textured().model(buffers.modelMatrixF ? buffers.modelMatrixF() : glm::mat4(1.0f));
			Tools::TexturedRender(Globals::Shaders().textured(), buffers, buffers.texture);
		};

		for (const auto& buffers : staticBuffers.texturedWalls)
			render(buffers);

		for (const auto& buffers : staticBuffers.texturedGrapples)
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers.texturedWalls)
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers.texturedGrapples)
			render(buffers);
	}

	void Walls::basicRender() const
	{
		glUseProgram_proxy(Globals::Shaders().basic().getProgramId());
		Globals::Shaders().basic().vp(Globals::Components().mvp().getVP());

		auto render = [&](const auto& buffers)
		{
			Globals::Shaders().basic().color(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().basic().model(buffers.modelMatrixF ? buffers.modelMatrixF() : glm::mat4(1.0f));

			std::function<void()> renderingTeardown =
				Globals::Components().renderingSetups()[buffers.renderingSetup](Globals::Shaders().basic().getProgramId());

			buffers.draw();

			if (renderingTeardown)
				renderingTeardown();
		};

		for (const auto& buffers : staticBuffers.simpleWalls)
			render(buffers);

		for (const auto& buffers : staticBuffers.simpleGrapples)
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers.simpleWalls)
			render(buffers);

		for (const auto& [id, buffers] : dynamicBuffers.simpleGrapples)
			render(buffers);
	}
}
