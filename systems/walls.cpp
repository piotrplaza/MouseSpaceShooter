#include "walls.hpp"

#include <ogl/oglProxy.hpp>
#include <ogl/buffersHelpers.hpp>
#include <ogl/renderingHelpers.hpp>

#include <ogl/shaders/basic.hpp>
#include <ogl/shaders/sceneCoordTextured.hpp>
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
		updateDynamicWallsPositionsBuffers();
		updateGrapplesPositionsBuffers();
	}

	void Walls::render() const
	{
		basicRender();
		texturedRender();
		sceneCoordTexturedRender();
		customShadersRender();
	}

	void Walls::initGraphics()
	{
		simpleStaticWallsBuffers = std::make_unique<Buffers::PosTexCoordBuffers>();
		simpleDynamicWallsBuffers = std::make_unique<Buffers::PosTexCoordBuffers>();
		simpleGrapplesBuffers = std::make_unique<Buffers::PosTexCoordBuffers>();

		updateStaticWallsPositionsBuffers();
		updatePersistentTexCoordsBuffers();
	}

	void Walls::updateStaticWallsPositionsBuffers()
	{
		Tools::UpdateTransformedPositionsBuffers(Globals::Components().staticWalls(),
			*simpleStaticWallsBuffers, texturesToStaticWallsBuffers, customSimpleStaticWallsBuffers,
			customTexturedStaticWallsBuffers, customShadersStaticWallsBuffers);
	}

	void Walls::updateDynamicWallsPositionsBuffers()
	{
		Tools::UpdateTransformedPositionsBuffers(Globals::Components().dynamicWalls(),
			*simpleDynamicWallsBuffers, texturesToDynamicWallsBuffers, customSimpleDynamicWallsBuffers,
			customTexturedDynamicWallsBuffers, customShadersDynamicWallsBuffers);
	}

	void Walls::updateGrapplesPositionsBuffers()
	{
		Tools::UpdateTransformedPositionsBuffers(Globals::Components().grapples(),
			*simpleGrapplesBuffers, texturesToGrapplesBuffers, customSimpleGrapplesBuffers,
			customTexturedGrapplesBuffers, customShadersGrapplesBuffers);
	}

	void Walls::updatePersistentTexCoordsBuffers()
	{
		updateDynamicWallsTexCoordsBuffers();
		updateGrapplesTexCoordsBuffers();
	}

	void Walls::updateDynamicWallsTexCoordsBuffers()
	{
		Tools::UpdateTexCoordBuffers(Globals::Components().dynamicWalls(), texturesToDynamicWallsBuffers,
			customTexturedDynamicWallsBuffers, customShadersDynamicWallsBuffers);
	}

	void Walls::updateGrapplesTexCoordsBuffers()
	{
		Tools::UpdateTexCoordBuffers(Globals::Components().grapples(), texturesToGrapplesBuffers,
			customTexturedGrapplesBuffers, customShadersGrapplesBuffers);
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

			glBindVertexArray(currentBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, currentBuffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		}
	}

	void Walls::customShadersRender() const
	{
		customShadersRender(customShadersDynamicWallsBuffers);
		customShadersRender(customShadersStaticWallsBuffers);
		customShadersRender(customShadersGrapplesBuffers);
	}

	void Walls::sceneCoordTexturedRender() const
	{
		glUseProgram_proxy(Globals::Shaders().sceneCoordTextured().getProgramId());
		Globals::Shaders().sceneCoordTextured().vp(Globals::Components().mvp().getVP());

		for (const auto& [texture, texturedStaticWallBuffers] : texturesToStaticWallsBuffers)
		{
			Globals::Shaders().sceneCoordTextured().color(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().sceneCoordTextured().model(glm::mat4(1.0f));
			Tools::TexturedRender(Globals::Shaders().sceneCoordTextured(), texturedStaticWallBuffers, texture);
		}

		for (const auto& customTexturedStaticWallBuffers : customTexturedStaticWallsBuffers)
		{
			Globals::Shaders().sceneCoordTextured().color(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().sceneCoordTextured().model(glm::mat4(1.0f));
			Tools::TexturedRender(Globals::Shaders().sceneCoordTextured(), customTexturedStaticWallBuffers,
				customTexturedStaticWallBuffers.texture);
		}
	}

	void Walls::texturedRender() const
	{
		glUseProgram_proxy(Globals::Shaders().textured().getProgramId());
		Globals::Shaders().textured().vp(Globals::Components().mvp().getVP());

		for (const auto& [texture, texturedDynamicWallBuffers] : texturesToDynamicWallsBuffers)
		{
			Globals::Shaders().textured().color(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().textured().model(glm::mat4(1.0f));
			Tools::TexturedRender(Globals::Shaders().textured(), texturedDynamicWallBuffers, texture);
		}

		for (const auto& [texture, texturedGrappleBuffers] : texturesToGrapplesBuffers)
		{
			Globals::Shaders().textured().color(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().textured().model(glm::mat4(1.0f));
			Tools::TexturedRender(Globals::Shaders().textured(), texturedGrappleBuffers, texture);
		}

		for (const auto& customTexturedDynamicWallBuffers : customTexturedDynamicWallsBuffers)
		{
			Globals::Shaders().textured().color(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().textured().model(glm::mat4(1.0f));
			Tools::TexturedRender(Globals::Shaders().textured(), customTexturedDynamicWallBuffers,
				customTexturedDynamicWallBuffers.texture);
		}

		for (const auto& customTextureGrappleBuffers : customTexturedGrapplesBuffers)
		{
			Globals::Shaders().textured().color(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().textured().model(glm::mat4(1.0f));
			Tools::TexturedRender(Globals::Shaders().textured(), customTextureGrappleBuffers,
				customTextureGrappleBuffers.texture);
		}
	}

	void Walls::basicRender() const
	{
		glUseProgram_proxy(Globals::Shaders().basic().getProgramId());
		Globals::Shaders().basic().vp(Globals::Components().mvp().getVP());
		Globals::Shaders().basic().color(Globals::Components().graphicsSettings().defaultColor);
		Globals::Shaders().basic().model(glm::mat4(1.0f));

		glBindVertexArray(simpleStaticWallsBuffers->vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, simpleStaticWallsBuffers->positionsCache.size());

		glBindVertexArray(simpleDynamicWallsBuffers->vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, simpleDynamicWallsBuffers->positionsCache.size());

		glBindVertexArray(simpleGrapplesBuffers->vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, simpleGrapplesBuffers->positionsCache.size());

		for (const auto& customSimpleStaticWallBuffers : customSimpleStaticWallsBuffers)
		{
			Globals::Shaders().basic().color(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().basic().model(glm::mat4(1.0f));

			std::function<void()> renderingTeardown =
				Globals::Components().renderingSetups()[customSimpleStaticWallBuffers.renderingSetup](Globals::Shaders().basic().getProgramId());

			glBindVertexArray(customSimpleStaticWallBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, customSimpleStaticWallBuffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		}

		for (const auto& customSimpleDynamicWallBuffers : customSimpleDynamicWallsBuffers)
		{
			Globals::Shaders().basic().color(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().basic().model(glm::mat4(1.0f));

			std::function<void()> renderingTeardown =
				Globals::Components().renderingSetups()[customSimpleDynamicWallBuffers.renderingSetup](Globals::Shaders().basic().getProgramId());

			glBindVertexArray(customSimpleDynamicWallBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, customSimpleDynamicWallBuffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		}

		for (const auto& customSimpleGrappleBuffers : customSimpleGrapplesBuffers)
		{
			Globals::Shaders().basic().color(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().basic().model(glm::mat4(1.0f));

			std::function<void()> renderingTeardown =
				Globals::Components().renderingSetups()[customSimpleGrappleBuffers.renderingSetup](Globals::Shaders().basic().getProgramId());

			glBindVertexArray(customSimpleGrappleBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, customSimpleGrappleBuffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		}
	}
}
