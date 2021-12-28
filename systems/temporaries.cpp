#include "temporaries.hpp"

#include <ogl/oglProxy.hpp>
#include <ogl/buffersHelpers.hpp>
#include <ogl/renderingHelpers.hpp>

#include <ogl/shaders/basic.hpp>
#include <ogl/shaders/textured.hpp>

#include <globals.hpp>

#include <components/missile.hpp>
#include <components/decoration.hpp>
#include <components/renderingSetup.hpp>
#include <components/mvp.hpp>
#include <components/graphicsSettings.hpp>

namespace Systems
{
	Temporaries::Temporaries() = default;

	void Temporaries::step()
	{
		for (auto& [id, missile] : Globals::Components().missiles())
			if (missile.step)
				missile.step();

		updatePosAndTexCoordBuffers();
	}

	void Temporaries::render() const
	{
		basicRender(simpleRocketsBuffers);
		texturedRender(texturedRocketsBuffers);
		customShadersRender(customShaderRocketsBuffers);
	}

	void Temporaries::updatePosAndTexCoordBuffers()
	{
		Tools::UpdatePosTexCoordBuffers(Globals::Components().missiles(), simpleRocketsBuffers,
			texturedRocketsBuffers, customShaderRocketsBuffers);
	}

	void Temporaries::customShadersRender(const std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers>& buffers) const
	{
		for (const auto& [id, currentBuffers] : buffers)
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

	void Temporaries::texturedRender(const std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers>& buffers) const
	{
		glUseProgram_proxy(Globals::Shaders().textured().getProgramId());
		Globals::Shaders().textured().vpUniform.setValue(Globals::Components().mvp().getVP());

		for (const auto& [id, currentBuffers] : buffers)
		{
			Globals::Shaders().textured().colorUniform.setValue(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().textured().modelUniform.setValue(glm::mat4(1.0f));
			Tools::TexturedRender(Globals::Shaders().textured(), currentBuffers, currentBuffers.texture);
		}
	}

	void Temporaries::basicRender(const std::unordered_map<ComponentId, Buffers::PosTexCoordBuffers>& buffers) const
	{
		glUseProgram_proxy(Globals::Shaders().basic().getProgramId());
		Globals::Shaders().basic().vpUniform.setValue(Globals::Components().mvp().getVP());

		for (const auto& [id, currentBuffers] : buffers)
		{
			Globals::Shaders().basic().colorUniform.setValue(Globals::Components().graphicsSettings().defaultColor);
			Globals::Shaders().basic().modelUniform.setValue(glm::mat4(1.0f));

			std::function<void()> renderingTeardown;
			if (currentBuffers.renderingSetup)
				renderingTeardown = Globals::Components().renderingSetups()[currentBuffers.renderingSetup](Globals::Shaders().basic().getProgramId());

			glBindVertexArray(currentBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, currentBuffers.positionsCache.size());

			if (renderingTeardown)
				renderingTeardown();
		}
	}
}
