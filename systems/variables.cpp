#include "variables.hpp"

#include <glm/gtc/constants.hpp>

#include <ogl/oglProxy.hpp>
#include <ogl/buffersHelpers.hpp>
#include <ogl/renderingHelpers.hpp>

#include <globals.hpp>

#include <components/rocket.hpp>
#include <components/mvp.hpp>
#include <components/graphicsSettings.hpp>
#include <components/mouseState.hpp>

#include <tools/b2Helpers.hpp>
#include <tools/utility.hpp>

namespace Systems
{
	Variables::Variables()
	{
		initGraphics();
	}

	void Variables::initGraphics()
	{
		basicShadersProgram = std::make_unique<Shaders::Programs::Basic>();
		texturedShadersProgram = std::make_unique<Shaders::Programs::Textured>();
	}

	void Variables::updatePositionsBuffers()
	{
		Tools::UpdatePositionsBuffers(Globals::Components::rockets, simpleRocketsBuffers,
			texturedRocketsBuffers, customShaderRocketsBuffers, GL_DYNAMIC_DRAW);
	}

	void Variables::updateTexCoordsBuffers()
	{
		Tools::UpdateTexCoordBuffers(Globals::Components::rockets, texturedRocketsBuffers,
			customShaderRocketsBuffers, GL_DYNAMIC_DRAW);
	}

	void Variables::customShadersRender(const std::vector<Buffers::PosTexCoordBuffers>& buffers) const
	{
		for (const auto& currentBuffers : buffers)
		{
			assert(currentBuffers.customShadersProgram);
			glUseProgram_proxy(*currentBuffers.customShadersProgram);

			std::function<void()> renderingTeardown;
			if (currentBuffers.renderingSetup)
				renderingTeardown = currentBuffers.renderingSetup(*currentBuffers.customShadersProgram);

			glBindVertexArray(currentBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, currentBuffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		}
	}

	void Variables::texturedRender(const std::vector<Buffers::PosTexCoordBuffers>& buffers) const
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

	void Variables::basicRender(const std::vector<Buffers::PosTexCoordBuffers>& buffers) const
	{
		glUseProgram_proxy(basicShadersProgram->program);
		basicShadersProgram->vpUniform.setValue(Globals::Components::mvp.getVP());

		for (const auto& currentBuffers : buffers)
		{
			basicShadersProgram->colorUniform.setValue(Globals::Components::graphicsSettings.defaultColor);
			basicShadersProgram->modelUniform.setValue(glm::mat4(1.0f));

			std::function<void()> renderingTeardown;
			if (currentBuffers.renderingSetup)
				renderingTeardown = currentBuffers.renderingSetup(basicShadersProgram->program);

			glBindVertexArray(currentBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, currentBuffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		}
	}

	void Variables::step()
	{
		updatePositionsBuffers();
		updateTexCoordsBuffers();

		static int counter = 0;
		if (counter++ % 100 == 0)
		{
			Globals::Components::rockets.emplace_back(Tools::CreateBoxBody({ 0.0f, 0.0f }, { 0.5f, 0.5f }, Tools::Random(0.0f, glm::two_pi<float>()), b2_dynamicBody));
			Globals::Components::rockets.back().renderingSetup = [modelUniform = Uniforms::UniformControllerMat4f(),
				&body = *Globals::Components::rockets.back().body](Shaders::ProgramId program) mutable
			{
				if (!modelUniform.isValid()) modelUniform = Uniforms::UniformControllerMat4f(program, "model");
				modelUniform.setValue(Tools::GetModelMatrix(body));
				return nullptr;
			};
		}
	}

	void Variables::render() const
	{
		customShadersRender(customShaderRocketsBuffers);
		texturedRender(texturedRocketsBuffers);
		basicRender(simpleRocketsBuffers);
	}
}
