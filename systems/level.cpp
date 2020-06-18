#include "level.hpp"

#include <Box2D/Box2D.h>

#include <glm/gtc/type_ptr.hpp>

#include <globals.hpp>

#include <components/physics.hpp>
#include <components/mvp.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>

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

		shadersProgram = shaders::LinkProgram(shaders::CompileShaders("shaders/basic.vs", "shaders/basic.fs"),
			{ {0, "bPos"} });

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

	void Level::step()
	{
		updateWalls();
		updateGrapples();
	}

	void Level::render() const
	{
		glUseProgram(shadersProgram);
		glUniformMatrix4fv(glGetUniformLocation(shadersProgram, "mvp"), 1, GL_FALSE,
			glm::value_ptr(Globals::Components::mvp.getVP()));

		glBindVertexArray(wallsVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, wallsVerticesCache.size());

		glBindVertexArray(grapplesVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, grapplesVerticesCache.size());
	}
}
