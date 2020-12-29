#include "temporaries.hpp"

#include <glm/gtc/constants.hpp>

#include <ogl/oglProxy.hpp>
#include <ogl/buffersHelpers.hpp>
#include <ogl/renderingHelpers.hpp>

#include <globals.hpp>

#include <components/rocket.hpp>
#include <components/mvp.hpp>
#include <components/graphicsSettings.hpp>
#include <components/mouseState.hpp>
#include <components/player.hpp>

#include <tools/b2Helpers.hpp>
#include <tools/utility.hpp>
#include <tools/gameHelpers.hpp>

namespace Systems
{
	Temporaries::Temporaries()
	{
		initGraphics();
	}

	void Temporaries::initGraphics()
	{
		basicShadersProgram = std::make_unique<Shaders::Programs::Basic>();
		texturedShadersProgram = std::make_unique<Shaders::Programs::Textured>();
	}

	void Temporaries::updatePositionsBuffers()
	{
		Tools::UpdatePositionsBuffers(Globals::Components::rockets, simpleRocketsBuffers,
			texturedRocketsBuffers, customShaderRocketsBuffers, GL_DYNAMIC_DRAW);
	}

	void Temporaries::updateTexCoordsBuffers()
	{
		Tools::UpdateTexCoordBuffers(Globals::Components::rockets, texturedRocketsBuffers,
			customShaderRocketsBuffers, GL_DYNAMIC_DRAW);
	}

	void Temporaries::customShadersRender(const std::vector<Buffers::PosTexCoordBuffers>& buffers) const
	{
		for (const auto& currentBuffers : buffers)
		{
			assert(currentBuffers.customShadersProgram);
			glUseProgram_proxy(*currentBuffers.customShadersProgram);

			std::function<void()> renderingTeardown;
			if (currentBuffers.renderingSetup)
				renderingTeardown = (*currentBuffers.renderingSetup)(*currentBuffers.customShadersProgram);

			glBindVertexArray(currentBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, currentBuffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		}
	}

	void Temporaries::texturedRender(const std::vector<Buffers::PosTexCoordBuffers>& buffers) const
	{
		glUseProgram_proxy(texturedShadersProgram->program);
		texturedShadersProgram->vpUniform.setValue(Globals::Components::mvp.getVP());

		for (const auto& currentBuffers : buffers)
		{
			texturedShadersProgram->colorUniform.setValue(Globals::Components::graphicsSettings.defaultColor);
			texturedShadersProgram->modelUniform.setValue(glm::mat4(1.0f));
			Tools::TexturedRender(*texturedShadersProgram, currentBuffers, *currentBuffers.texture);
		}
	}

	void Temporaries::basicRender(const std::vector<Buffers::PosTexCoordBuffers>& buffers) const
	{
		glUseProgram_proxy(basicShadersProgram->program);
		basicShadersProgram->vpUniform.setValue(Globals::Components::mvp.getVP());

		for (const auto& currentBuffers : buffers)
		{
			basicShadersProgram->colorUniform.setValue(Globals::Components::graphicsSettings.defaultColor);
			basicShadersProgram->modelUniform.setValue(glm::mat4(1.0f));

			std::function<void()> renderingTeardown;
			if (currentBuffers.renderingSetup)
				renderingTeardown = (*currentBuffers.renderingSetup)(basicShadersProgram->program);

			glBindVertexArray(currentBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, currentBuffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		}
	}

	void Temporaries::step()
	{
		updatePositionsBuffers();
		updateTexCoordsBuffers();
	}

	void Temporaries::render() const
	{
		customShadersRender(customShaderRocketsBuffers);
		texturedRender(texturedRocketsBuffers);
		basicRender(simpleRocketsBuffers);
	}
}
