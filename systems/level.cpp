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

		basicShadersProgram = shaders::LinkProgram(shaders::CompileShaders("shaders/basic.vs", "shaders/basic.fs"),
			{ {0, "bPos"} });
		basicShadersMVPUniform = glGetUniformLocation(basicShadersProgram, "mvp");
		basicShadersColorUniform = glGetUniformLocation(basicShadersProgram, "color");

		coloredShadersProgram = shaders::LinkProgram(shaders::CompileShaders("shaders/colored.vs", "shaders/colored.fs"),
			{ {0, "bPos"}, {1, "bColor"} });
		coloredShadersMVPUniform = glGetUniformLocation(coloredShadersProgram, "mvp");

		sceneCoordTexturedShadersProgram = shaders::LinkProgram(shaders::CompileShaders("shaders/sceneCoordTextured.vs", "shaders/sceneCoordTextured.fs"),
			{ {0, "bPos"} });
		sceneCoordTexturedShadersMVPUniform = glGetUniformLocation(sceneCoordTexturedShadersProgram, "mvp");
		sceneCoordTexturedShadersModelUniform = glGetUniformLocation(sceneCoordTexturedShadersProgram, "model");
		sceneCoordTexturedShadersTexture1Uniform = glGetUniformLocation(sceneCoordTexturedShadersProgram, "texture1");
		sceneCoordTexturedShadersTextureScalingUniform = glGetUniformLocation(sceneCoordTexturedShadersProgram, "textureScaling");

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

		glCreateVertexArrays(1, &connectionsVertexArray);
		glBindVertexArray(connectionsVertexArray);
		glGenBuffers(1, &connectionsVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, connectionsVertexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);
		glGenBuffers(1, &connectionsColorBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, connectionsColorBuffer);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(1);

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

	void Level::updateConnectionsGraphics()
	{
		using namespace Globals::Components;

		connectionsVerticesCache.clear();
		connectionsColorsCache.clear();
		for (auto& connection : connections)
		{
			connection.updateVerticesCache();
			connectionsVerticesCache.insert(connectionsVerticesCache.end(), connection.verticesCache.begin(), connection.verticesCache.end());

			connection.updateColorsCache();
			connectionsColorsCache.insert(connectionsColorsCache.end(), connection.colorsCache.begin(), connection.colorsCache.end());
		}

		glBindBuffer(GL_ARRAY_BUFFER, connectionsVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, connectionsVerticesCache.size() * sizeof(connectionsVerticesCache.front()),
			connectionsVerticesCache.data(), GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, connectionsColorBuffer);
		glBufferData(GL_ARRAY_BUFFER, connectionsColorsCache.size() * sizeof(connectionsColorsCache.front()),
			connectionsColorsCache.data(), GL_DYNAMIC_DRAW);
	}

	void Level::step()
	{
		updateDynamicWallsGraphics();
		updateGrapplesGraphics();
		updateConnectionsGraphics();
	}

	void Level::renderBackground() const
	{
		using namespace Globals::Components;
		using namespace Globals::Constants;

		glUseProgram(sceneCoordTexturedShadersProgram);
		glUniformMatrix4fv(sceneCoordTexturedShadersMVPUniform, 1, GL_FALSE,
			glm::value_ptr(Globals::Components::mvp.getVP()));
		glUniformMatrix4fv(sceneCoordTexturedShadersModelUniform, 1, GL_FALSE,
			glm::value_ptr(glm::mat4(1.0f)));

		//TODO: Improve performance by preallocate VRAM buffers.
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glEnableVertexAttribArray(0);
		for (const auto& [texture, texturedStaticWallsVerticesCache] : textureToStaticWallsVerticesCache)
		{
			const auto& textureComponent = textures[texture];
			//const float textureScale

			glUniform1i(sceneCoordTexturedShadersTexture1Uniform, texture);
			glUniform2f(sceneCoordTexturedShadersTextureScalingUniform,
				(float)textureComponent.height / textureComponent.width * defaultScreenCoordTextureScaling, defaultScreenCoordTextureScaling);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, texturedStaticWallsVerticesCache.data());
			glDrawArrays(GL_TRIANGLES, 0, texturedStaticWallsVerticesCache.size());
		}
		//

		glUseProgram(basicShadersProgram);
		glUniformMatrix4fv(basicShadersMVPUniform, 1, GL_FALSE,
			glm::value_ptr(Globals::Components::mvp.getVP()));

		glUniform4f(basicShadersColorUniform, 0.5f, 0.5f, 0.5f, 1.0f);
		glBindVertexArray(staticWallsVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, simpleStaticWallsVerticesCache.size());

		glUniform4f(basicShadersColorUniform, 0.5f, 0.5f, 0.5f, 1.0f);
		glBindVertexArray(dynamicWallsVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, simpleDynamicWallsVerticesCache.size());

		glUniform4f(basicShadersColorUniform, 0.0f, 0.5f, 0.0f, 1.0f);
		glBindVertexArray(grapplesVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, grapplesVerticesCache.size());
	}

	void Level::renderForeground() const
	{
		glUseProgram(coloredShadersProgram);
		glUniformMatrix4fv(coloredShadersMVPUniform, 1, GL_FALSE,
			glm::value_ptr(Globals::Components::mvp.getVP()));

		glBindVertexArray(connectionsVertexArray);
		glDrawArrays(GL_LINES, 0, connectionsVerticesCache.size());
	}
}
