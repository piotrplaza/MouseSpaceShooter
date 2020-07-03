#include "level.hpp"

#include <Box2D/Box2D.h>

#include <glm/gtc/type_ptr.hpp>

#include <globals.hpp>

#include <components/physics.hpp>
#include <components/mvp.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/connection.hpp>
#include <components/texture.hpp>

namespace Systems
{
	Level::Level()
	{
		initGraphics();
	}

	void Level::initGraphics()
	{
		using namespace Globals::Components;

		basicShadersProgram.program = Shaders::LinkProgram(Shaders::CompileShaders("shaders/basic.vs", "shaders/basic.fs"),
			{ {0, "bPos"} });
		basicShadersProgram.mvpUniform = glGetUniformLocation(basicShadersProgram.program, "mvp");
		basicShadersProgram.colorUniform = glGetUniformLocation(basicShadersProgram.program, "color");

		sceneCoordTextured.program = Shaders::LinkProgram(Shaders::CompileShaders("shaders/sceneCoordTextured.vs", "shaders/sceneCoordTextured.fs"),
			{ {0, "bPos"} });
		sceneCoordTextured.mvpUniform = glGetUniformLocation(sceneCoordTextured.program, "mvp");
		sceneCoordTextured.modelUniform = glGetUniformLocation(sceneCoordTextured.program, "model");
		sceneCoordTextured.texture1Uniform = glGetUniformLocation(sceneCoordTextured.program, "texture1");
		sceneCoordTextured.textureScalingUniform = glGetUniformLocation(sceneCoordTextured.program, "textureScaling");

		glCreateVertexArrays(1, &staticWallsVertexArray);
		glBindVertexArray(staticWallsVertexArray);
		glGenBuffers(1, &staticWallsVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, staticWallsVertexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);

		glCreateVertexArrays(1, &dynamicWallsVertexArray);
		glBindVertexArray(dynamicWallsVertexArray);
		glGenBuffers(1, &dynamicWallsVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, dynamicWallsVertexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);

		glCreateVertexArrays(1, &grapplesVertexArray);
		glBindVertexArray(grapplesVertexArray);
		glGenBuffers(1, &grapplesVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, grapplesVertexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);

		updateStaticWallsGraphics();
	}

	void Level::updateStaticWallsGraphics()
	{
		using namespace Globals::Components;

		updateWallsVerticesCache(staticWalls, simpleStaticWallsVerticesCache, textureToStaticWallsVerticesCache);

		glBindBuffer(GL_ARRAY_BUFFER, staticWallsVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, simpleStaticWallsVerticesCache.size() * sizeof(simpleStaticWallsVerticesCache.front()),
			simpleStaticWallsVerticesCache.data(), GL_STATIC_DRAW);
	}

	void Level::updateDynamicWallsGraphics()
	{
		using namespace Globals::Components;

		updateWallsVerticesCache(dynamicWalls, simpleDynamicWallsVerticesCache, textureToDynamicWallsVerticesCache);

		glBindBuffer(GL_ARRAY_BUFFER, dynamicWallsVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, simpleDynamicWallsVerticesCache.size() * sizeof(simpleDynamicWallsVerticesCache.front()),
			simpleDynamicWallsVerticesCache.data(), GL_DYNAMIC_DRAW);
	}

	void Level::updateWallsVerticesCache(std::vector<::Components::Wall>& walls, std::vector<glm::vec3>& simpleWallsVerticesCache,
		std::unordered_map<unsigned, std::vector<glm::vec3>>& textureToWallsVerticesCache) const
	{
		simpleWallsVerticesCache.clear();
		textureToWallsVerticesCache.clear();
		for (auto& wall : walls)
		{
			wall.updateVerticesCache();
			if (wall.texture)
			{
				auto& texturedWallVerticesCache = textureToWallsVerticesCache[*wall.texture];
				texturedWallVerticesCache.insert(texturedWallVerticesCache.end(), wall.verticesCache.begin(), wall.verticesCache.end());
			}
			else
			{
				simpleWallsVerticesCache.insert(simpleWallsVerticesCache.end(), wall.verticesCache.begin(), wall.verticesCache.end());
			}
		}
	}

	void Level::updateGrapplesGraphics()
	{
		using namespace Globals::Components;

		grapplesVerticesCache.clear();
		for (auto& grapple : grapples)
		{
			grapple.updateVerticesCache();
			grapplesVerticesCache.insert(grapplesVerticesCache.end(), grapple.verticesCache.begin(), grapple.verticesCache.end());
		}

		glBindBuffer(GL_ARRAY_BUFFER, grapplesVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, grapplesVerticesCache.size() * sizeof(grapplesVerticesCache.front()),
			grapplesVerticesCache.data(), GL_DYNAMIC_DRAW);
	}

	void Level::step()
	{
		updateDynamicWallsGraphics();
		updateGrapplesGraphics();
	}

	void Level::render() const
	{
		using namespace Globals::Components;
		using namespace Globals::Constants;

		glUseProgram(sceneCoordTextured.program);
		glUniformMatrix4fv(sceneCoordTextured.mvpUniform, 1, GL_FALSE,
			glm::value_ptr(Globals::Components::mvp.getVP()));
		glUniformMatrix4fv(sceneCoordTextured.modelUniform, 1, GL_FALSE,
			glm::value_ptr(glm::mat4(1.0f)));

		//TODO: Improve performance by preallocate VRAM buffers.
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glEnableVertexAttribArray(0);
		for (const auto& [texture, texturedStaticWallsVerticesCache] : textureToStaticWallsVerticesCache)
		{
			const auto& textureComponent = textures[texture];

			glUniform1i(sceneCoordTextured.texture1Uniform, texture);
			glUniform2f(sceneCoordTextured.textureScalingUniform,
				(float)textureComponent.height / textureComponent.width * defaultScreenCoordTextureScaling, defaultScreenCoordTextureScaling);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, texturedStaticWallsVerticesCache.data());
			glDrawArrays(GL_TRIANGLES, 0, texturedStaticWallsVerticesCache.size());
		}
		//

		glUseProgram(basicShadersProgram.program);
		glUniformMatrix4fv(basicShadersProgram.mvpUniform, 1, GL_FALSE,
			glm::value_ptr(Globals::Components::mvp.getVP()));

		glUniform4f(basicShadersProgram.colorUniform, 0.5f, 0.5f, 0.5f, 1.0f);
		glBindVertexArray(staticWallsVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, simpleStaticWallsVerticesCache.size());

		glUniform4f(basicShadersProgram.colorUniform, 0.5f, 0.5f, 0.5f, 1.0f);
		glBindVertexArray(dynamicWallsVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, simpleDynamicWallsVerticesCache.size());

		glUniform4f(basicShadersProgram.colorUniform, 0.0f, 0.5f, 0.0f, 1.0f);
		glBindVertexArray(grapplesVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, grapplesVerticesCache.size());
	}
}
