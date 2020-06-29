#include "level.hpp"

#include <Box2D/Box2D.h>

#include <glm/gtc/type_ptr.hpp>

#include <globals.hpp>

#include <components/physics.hpp>
#include <components/mvp.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/connection.hpp>

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

		staticWallsVerticesCache.clear();
		for (auto& staticWall : staticWalls)
		{
			staticWall.updateVerticesCache();
			staticWallsVerticesCache.insert(staticWallsVerticesCache.end(), staticWall.verticesCache.begin(), staticWall.verticesCache.end());
		}

		glBindBuffer(GL_ARRAY_BUFFER, staticWallsVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, staticWallsVerticesCache.size() * sizeof(staticWallsVerticesCache.front()),
			staticWallsVerticesCache.data(), GL_STATIC_DRAW);
	}

	void Level::updateDynamicWallsGraphics()
	{
		using namespace Globals::Components;

		dynamicWallsVerticesCache.clear();
		for (auto& dynamicWall : dynamicWalls)
		{
			dynamicWall.updateVerticesCache();
			dynamicWallsVerticesCache.insert(dynamicWallsVerticesCache.end(), dynamicWall.verticesCache.begin(), dynamicWall.verticesCache.end());
		}

		glBindBuffer(GL_ARRAY_BUFFER, dynamicWallsVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, dynamicWallsVerticesCache.size() * sizeof(dynamicWallsVerticesCache.front()),
			dynamicWallsVerticesCache.data(), GL_DYNAMIC_DRAW);
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
		glUseProgram(basicShadersProgram);
		glUniformMatrix4fv(basicShadersMVPUniform, 1, GL_FALSE,
			glm::value_ptr(Globals::Components::mvp.getVP()));

		glUniform4f(basicShadersColorUniform, 0.5f, 0.5f, 0.5f, 1.0f);
		glBindVertexArray(staticWallsVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, staticWallsVerticesCache.size());

		glUniform4f(basicShadersColorUniform, 0.5f, 0.5f, 0.5f, 1.0f);
		glBindVertexArray(dynamicWallsVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, dynamicWallsVerticesCache.size());

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
