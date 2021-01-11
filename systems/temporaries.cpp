#include "temporaries.hpp"

#include <ogl/oglProxy.hpp>
#include <ogl/buffersHelpers.hpp>
#include <ogl/renderingHelpers.hpp>

#include <globals.hpp>

#include <components/missile.hpp>
#include <components/decoration.hpp>
#include <components/mvp.hpp>
#include <components/graphicsSettings.hpp>

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

	void Temporaries::updatePosAndTexCoordBuffers()
	{
		Tools::UpdatePosTexCoordBuffers(Globals::Components::missiles, simpleRocketsBuffers,
			texturedRocketsBuffers, customShaderRocketsBuffers, GL_STATIC_DRAW);
	}

	void Temporaries::customShadersRender(const std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers>& buffers) const
	{
		for (const auto& [id, currentBuffers] : buffers)
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

	void Temporaries::texturedRender(const std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers>& buffers) const
	{
		glUseProgram_proxy(texturedShadersProgram->program);
		texturedShadersProgram->vpUniform.setValue(Globals::Components::mvp.getVP());

		for (const auto& [id, currentBuffers] : buffers)
		{
			texturedShadersProgram->colorUniform.setValue(Globals::Components::graphicsSettings.defaultColor);
			texturedShadersProgram->modelUniform.setValue(glm::mat4(1.0f));
			Tools::TexturedRender(*texturedShadersProgram, currentBuffers, *currentBuffers.texture);
		}
	}

	void Temporaries::basicRender(const std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers>& buffers) const
	{
		glUseProgram_proxy(basicShadersProgram->program);
		basicShadersProgram->vpUniform.setValue(Globals::Components::mvp.getVP());

		for (const auto& [id, currentBuffers] : buffers)
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
		for (auto& [id, missile] : Globals::Components::missiles)
			if (missile.step)
				missile.step();

		updatePosAndTexCoordBuffers();
	}

	void Temporaries::render() const
	{
		customShadersRender(customShaderRocketsBuffers);
		texturedRender(texturedRocketsBuffers);
		basicRender(simpleRocketsBuffers);
	}
}
