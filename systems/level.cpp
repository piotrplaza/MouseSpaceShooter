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
		initPhysics();
		initGraphics();
	}

	void Level::initPhysics() const
	{
		using namespace Globals::Components;
	}

	void Level::initGraphics()
	{
		using namespace Globals::Components;

		basicShadersProgram = shaders::LinkProgram(shaders::CompileShaders("shaders/basic.vs", "shaders/basic.fs"),
			{ {0, "bPos"} });

		coloredShadersProgram = shaders::LinkProgram(shaders::CompileShaders("shaders/colored.vs", "shaders/colored.fs"),
			{ {0, "bPos"}, {1, "bColor"} });

		glCreateVertexArrays(1, &wallsVertexArray);
		glBindVertexArray(wallsVertexArray);
		glGenBuffers(1, &wallsVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, wallsVertexBuffer);
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
	}

	void Level::updateWalls()
	{
		using namespace Globals::Components;

		wallsVerticesCache.clear();
		for (auto& wall : walls)
		{
			wall.updateVerticesCache();
			wallsVerticesCache.insert(wallsVerticesCache.end(), wall.verticesCache.begin(), wall.verticesCache.end());
		}

		glBindBuffer(GL_ARRAY_BUFFER, wallsVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, wallsVerticesCache.size() * sizeof(wallsVerticesCache.front()),
			wallsVerticesCache.data(), GL_DYNAMIC_DRAW);
	}

	void Level::updateGrapples()
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

	void Level::updateConnections()
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
		updateWalls();
		updateGrapples();
		updateConnections();
	}

	void Level::render() const
	{
		glUseProgram(basicShadersProgram);
		glUniformMatrix4fv(glGetUniformLocation(basicShadersProgram, "mvp"), 1, GL_FALSE,
			glm::value_ptr(Globals::Components::mvp.getVP()));

		glBindVertexArray(wallsVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, wallsVerticesCache.size());

		glBindVertexArray(grapplesVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, grapplesVerticesCache.size());


		glUseProgram(coloredShadersProgram);
		glUniformMatrix4fv(glGetUniformLocation(coloredShadersProgram, "mvp"), 1, GL_FALSE,
			glm::value_ptr(Globals::Components::mvp.getVP()));

		glBindVertexArray(connectionsVertexArray);
		glDrawArrays(GL_LINES, 0, connectionsVerticesCache.size());
	}
}
