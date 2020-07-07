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

		basicShadersProgram = std::make_unique<Shaders::Programs::Basic>();
		sceneCoordTexturedShadersProgram = std::make_unique<Shaders::Programs::SceneCoordTextured>();
		texturedShadersProgram = std::make_unique<Shaders::Programs::Textured>();

		staticWallsBuffers = std::make_unique<WallsBuffers>();
		dynamicWallsBuffers = std::make_unique<WallsBuffers>();
		grapplesBuffers = std::make_unique<GrapplesBuffers>();

		updateStaticWallsGraphics();
	}

	void Level::updateStaticWallsGraphics()
	{
		using namespace Globals::Components;

		updateWallsBuffers(staticWalls, *staticWallsBuffers, texturesToStaticWallsBuffers, GL_STATIC_DRAW);
	}

	void Level::updateDynamicWallsGraphics()
	{
		using namespace Globals::Components;

		updateWallsBuffers(dynamicWalls, *dynamicWallsBuffers, texturesToDynamicWallsBuffers, GL_DYNAMIC_DRAW);
	}

	void Level::updateWallsBuffers(std::vector<Components::Wall>& walls, WallsBuffers& simpleWallsBuffers,
		std::unordered_map<unsigned, WallsBuffers>& texturesToWallsBuffers, GLenum bufferDataUsage) const
	{
		simpleWallsBuffers.verticesCache.clear();
		for (auto& [texture, wallBuffers] : texturesToWallsBuffers)
		{
			wallBuffers.verticesCache.clear();
		}

		for (auto& wall : walls)
		{
			wall.updateVerticesCache();
			if (wall.texture)
			{
				auto& texturedWallBuffers = texturesToWallsBuffers[*wall.texture];
				texturedWallBuffers.verticesCache.insert(texturedWallBuffers.verticesCache.end(), wall.verticesCache.begin(), wall.verticesCache.end());
			}
			else
			{
				simpleWallsBuffers.verticesCache.insert(simpleWallsBuffers.verticesCache.end(), wall.verticesCache.begin(), wall.verticesCache.end());
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, simpleWallsBuffers.vertexBuffer);
		if (simpleWallsBuffers.vertexBufferAllocation < simpleWallsBuffers.verticesCache.size())
		{
			glBufferData(GL_ARRAY_BUFFER, simpleWallsBuffers.verticesCache.size() * sizeof(simpleWallsBuffers.verticesCache.front()),
				simpleWallsBuffers.verticesCache.data(), bufferDataUsage);
			simpleWallsBuffers.vertexBufferAllocation = simpleWallsBuffers.verticesCache.size();
		}
		else
		{
			glBufferSubData(GL_ARRAY_BUFFER, 0, simpleWallsBuffers.verticesCache.size() * sizeof(simpleWallsBuffers.verticesCache.front()),
				simpleWallsBuffers.verticesCache.data());
		}

		for (auto& [texture, texturedWallBuffers] : texturesToWallsBuffers)
		{
			glBindBuffer(GL_ARRAY_BUFFER, texturedWallBuffers.vertexBuffer);
			if (texturedWallBuffers.vertexBufferAllocation < texturedWallBuffers.verticesCache.size())
			{
				glBufferData(GL_ARRAY_BUFFER, texturedWallBuffers.verticesCache.size() * sizeof(texturedWallBuffers.verticesCache.front()),
					texturedWallBuffers.verticesCache.data(), bufferDataUsage);
				texturedWallBuffers.vertexBufferAllocation = texturedWallBuffers.verticesCache.size();
			}
			else
			{
				glBufferSubData(GL_ARRAY_BUFFER, 0, texturedWallBuffers.verticesCache.size() * sizeof(texturedWallBuffers.verticesCache.front()),
					texturedWallBuffers.verticesCache.data());
			}
		}
	}

	void Level::updateGrapplesGraphics()
	{
		using namespace Globals::Components;

		grapplesBuffers->verticesCache.clear();
		for (auto& grapple : grapples)
		{
			grapple.updateVerticesCache();
			grapplesBuffers->verticesCache.insert(grapplesBuffers->verticesCache.end(), grapple.verticesCache.begin(), grapple.verticesCache.end());
		}

		glBindBuffer(GL_ARRAY_BUFFER, grapplesBuffers->vertexBuffer);
		if (grapplesBuffers->vertexBufferAllocation < grapplesBuffers->verticesCache.size())
		{
			glBufferData(GL_ARRAY_BUFFER, grapplesBuffers->verticesCache.size() * sizeof(grapplesBuffers->verticesCache.front()),
				grapplesBuffers->verticesCache.data(), GL_DYNAMIC_DRAW);
			grapplesBuffers->vertexBufferAllocation = grapplesBuffers->verticesCache.size();
		}
		else
		{
			glBufferSubData(GL_ARRAY_BUFFER, 0, grapplesBuffers->verticesCache.size() * sizeof(grapplesBuffers->verticesCache.front()),
				grapplesBuffers->verticesCache.data());
		}
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

		glUseProgram(sceneCoordTexturedShadersProgram->program);
		glUniformMatrix4fv(sceneCoordTexturedShadersProgram->mvpUniform, 1, GL_FALSE,
			glm::value_ptr(Globals::Components::mvp.getVP()));
		glUniformMatrix4fv(sceneCoordTexturedShadersProgram->modelUniform, 1, GL_FALSE,
			glm::value_ptr(glm::mat4(1.0f)));

		for (const auto& [texture, texturedStaticWallBuffers] : texturesToStaticWallsBuffers)
		{
			const auto& textureComponent = textures[texture];

			glUniform1i(sceneCoordTexturedShadersProgram->texture1Uniform, texture);
			glUniform2f(sceneCoordTexturedShadersProgram->textureScaleUniform,
				(float)textureComponent.height / textureComponent.width * defaultScreenCoordTextureScaling, defaultScreenCoordTextureScaling);
			glBindVertexArray(texturedStaticWallBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, texturedStaticWallBuffers.verticesCache.size());
		}

		for (const auto& [texture, texturedDynamicWallBuffers] : texturesToDynamicWallsBuffers)
		{
			const auto& textureComponent = textures[texture];

			glUniform1i(sceneCoordTexturedShadersProgram->texture1Uniform, texture);
			glUniform2f(sceneCoordTexturedShadersProgram->textureScaleUniform,
				(float)textureComponent.height / textureComponent.width * defaultScreenCoordTextureScaling, defaultScreenCoordTextureScaling);
			glBindVertexArray(texturedDynamicWallBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, texturedDynamicWallBuffers.verticesCache.size());
		}

		glUseProgram(basicShadersProgram->program);
		glUniformMatrix4fv(basicShadersProgram->mvpUniform, 1, GL_FALSE,
			glm::value_ptr(Globals::Components::mvp.getVP()));

		glUniform4f(basicShadersProgram->colorUniform, 0.5f, 0.5f, 0.5f, 1.0f);
		glBindVertexArray(staticWallsBuffers->vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, staticWallsBuffers->verticesCache.size());

		glUniform4f(basicShadersProgram->colorUniform, 0.5f, 0.5f, 0.5f, 1.0f);
		glBindVertexArray(dynamicWallsBuffers->vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, dynamicWallsBuffers->verticesCache.size());

		glUniform4f(basicShadersProgram->colorUniform, 0.0f, 0.5f, 0.0f, 1.0f);
		glBindVertexArray(grapplesBuffers->vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, grapplesBuffers->verticesCache.size());
	}

	Level::WallsBuffers::WallsBuffers()
	{
		glCreateVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);
	}

	Level::WallsBuffers::~WallsBuffers()
	{
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteVertexArrays(1, &vertexArray);
	}

	Level::GrapplesBuffers::GrapplesBuffers()
	{
		glCreateVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);
	}

	Level::GrapplesBuffers::~GrapplesBuffers()
	{
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteVertexArrays(1, &vertexArray);
	}
}
