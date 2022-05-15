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
	}

	void Walls::render() const
	{
		basicRender();
		texturedRender();
		customShadersRender();
	}

	void Walls::initGraphics()
	{
		updateWallsPositionsBuffers();
		updateGrapplesPositionsBuffers();
		updateTexCoordsBuffers();
	}

	void Walls::updateWallsPositionsBuffers()
	{
		Tools::UpdatePositionsBuffers(Globals::Components().walls(), simpleWallsBuffers, texturedWallsBuffers, customShadersWallsBuffers);
	}

	void Walls::updateGrapplesPositionsBuffers()
	{
		Tools::UpdatePositionsBuffers(Globals::Components().grapples(), simpleGrapplesBuffers, texturedGrapplesBuffers, customShadersGrapplesBuffers);
	}

	void Walls::updateTexCoordsBuffers()
	{
		updateWallsTexCoordsBuffers();
		updateGrapplesTexCoordsBuffers();
	}

	void Walls::updateWallsTexCoordsBuffers()
	{
		Tools::UpdateTexCoordBuffers(Globals::Components().walls(), texturedWallsBuffers, customShadersWallsBuffers);
	}

	void Walls::updateGrapplesTexCoordsBuffers()
	{
		Tools::UpdateTexCoordBuffers(Globals::Components().grapples(), texturedGrapplesBuffers, customShadersGrapplesBuffers);
	}

	void Walls::customShadersRender(const std::vector<Buffers::PosTexCoordBuffers>& buffers) const
	{
		for (const auto& currentBuffers : buffers)
		{
			assert(currentBuffers.customShadersProgram);
			glUseProgram_proxy(*currentBuffers.customShadersProgram);

			std::function<void()> renderingTeardown;
			if (currentBuffers.renderingSetup)
				renderingTeardown = Globals::Components().renderingSetups()[currentBuffers.renderingSetup](*currentBuffers.customShadersProgram);

			currentBuffers.draw();

			if (renderingTeardown)
				renderingTeardown();
		}
	}

	void Walls::customShadersRender() const
	{
		customShadersRender(customShadersWallsBuffers);
		customShadersRender(customShadersGrapplesBuffers);
	}

	void Walls::texturedRender() const
	{
		glUseProgram_proxy(Globals::Shaders().textured().getProgramId());
		Globals::Shaders().textured().vp(Globals::Components().mvp().getVP());

		for (const auto& texturedWallBuffers : texturedWallsBuffers)
		{
			Globals::Shaders().textured().color(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().textured().model(texturedWallBuffers.modelMatrixF ? texturedWallBuffers.modelMatrixF() : glm::mat4(1.0f));
			Tools::TexturedRender(Globals::Shaders().textured(), texturedWallBuffers, texturedWallBuffers.texture);
		}

		for (const auto& textureGrappleBuffers : texturedGrapplesBuffers)
		{
			Globals::Shaders().textured().color(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().textured().model(textureGrappleBuffers.modelMatrixF ? textureGrappleBuffers.modelMatrixF() : glm::mat4(1.0f));
			Tools::TexturedRender(Globals::Shaders().textured(), textureGrappleBuffers, textureGrappleBuffers.texture);
		}
	}

	void Walls::basicRender() const
	{
		glUseProgram_proxy(Globals::Shaders().basic().getProgramId());
		Globals::Shaders().basic().vp(Globals::Components().mvp().getVP());
		Globals::Shaders().basic().color(Globals::Components().graphicsSettings().defaultColor);

		for (const auto& simpleWallBuffers : simpleWallsBuffers)
		{
			Globals::Shaders().basic().color(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().basic().model(simpleWallBuffers.modelMatrixF ? simpleWallBuffers.modelMatrixF() : glm::mat4(1.0f));

			std::function<void()> renderingTeardown =
				Globals::Components().renderingSetups()[simpleWallBuffers.renderingSetup](Globals::Shaders().basic().getProgramId());

			simpleWallBuffers.draw();

			if (renderingTeardown)
				renderingTeardown();
		}

		for (const auto& simpleGrappleBuffers : simpleGrapplesBuffers)
		{
			Globals::Shaders().basic().color(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().basic().model(simpleGrappleBuffers.modelMatrixF ? simpleGrappleBuffers.modelMatrixF() : glm::mat4(1.0f));

			std::function<void()> renderingTeardown =
				Globals::Components().renderingSetups()[simpleGrappleBuffers.renderingSetup](Globals::Shaders().basic().getProgramId());

			simpleGrappleBuffers.draw();

			if (renderingTeardown)
				renderingTeardown();
		}
	}
}
