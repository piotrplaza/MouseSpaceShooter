#include "level.hpp"

#include <Box2D/Box2D.h>

#include <ogl/oglProxy.hpp>

#include <globals.hpp>

#include <components/physics.hpp>
#include <components/mvp.hpp>
#include <components/wall.hpp>
#include <components/grapple.hpp>
#include <components/connection.hpp>
#include <components/texture.hpp>
#include <components/textureDef.hpp>

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

		simpleStaticWallsBuffers = std::make_unique<WallsBuffers>();
		simpleDynamicWallsBuffers = std::make_unique<WallsBuffers>();
		simpleGrapplesBuffers = std::make_unique<GrapplesBuffers>();

		updateStaticWallsGraphics();
		updateTexCoords();
	}

	void Level::updateStaticWallsGraphics()
	{
		using namespace Globals::Components;

		updateWallsBuffers(staticWalls, *simpleStaticWallsBuffers, texturesToStaticWallsBuffers, GL_STATIC_DRAW);
	}

	void Level::updateDynamicWallsGraphics()
	{
		using namespace Globals::Components;

		updateWallsBuffers(dynamicWalls, *simpleDynamicWallsBuffers, texturesToDynamicWallsBuffers, GL_DYNAMIC_DRAW);
	}

	void Level::updateTexCoords()
	{
		using namespace Globals::Components;

		for (auto& dynamicWall : dynamicWalls)
		{
			dynamicWall.updateVerticesCache(false);
			if (dynamicWall.texture)
			{
				auto& texturedDynamicWallBuffers = texturesToDynamicWallsBuffers.emplace(*dynamicWall.texture, true).first->second;
				for (const auto& vertex : dynamicWall.verticesCache)
				{
					texturedDynamicWallBuffers.texCoordCache.emplace_back(vertex);
				}
				glBindBuffer(GL_ARRAY_BUFFER, *texturedDynamicWallBuffers.texCoordBuffer);
				glBufferData(GL_ARRAY_BUFFER, texturedDynamicWallBuffers.texCoordCache.size() * sizeof(texturedDynamicWallBuffers.texCoordCache.front()),
					texturedDynamicWallBuffers.texCoordCache.data(), GL_STATIC_DRAW);
			}
		}

		for (auto& grapple : grapples)
		{
			grapple.updateVerticesCache(false);
			if (grapple.texture)
			{
				auto& texturedGrappleBuffers = texturesToGrapplesBuffers.emplace(*grapple.texture, true).first->second;
				for (const auto& vertex : grapple.verticesCache)
				{
					texturedGrappleBuffers.texCoordCache.emplace_back(vertex);
				}
				glBindBuffer(GL_ARRAY_BUFFER, *texturedGrappleBuffers.texCoordBuffer);
				glBufferData(GL_ARRAY_BUFFER, texturedGrappleBuffers.texCoordCache.size() * sizeof(texturedGrappleBuffers.texCoordCache.front()),
					texturedGrappleBuffers.texCoordCache.data(), GL_STATIC_DRAW);
			}
		}
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

		simpleGrapplesBuffers->verticesCache.clear();
		for (auto& [texture, grappleBuffers] : texturesToGrapplesBuffers)
		{
			grappleBuffers.verticesCache.clear();
		}

		for (auto& grapple : grapples)
		{
			grapple.updateVerticesCache();
			if (grapple.texture)
			{
				auto& texturedGrappleBuffers = texturesToGrapplesBuffers[*grapple.texture];
				texturedGrappleBuffers.verticesCache.insert(texturedGrappleBuffers.verticesCache.end(), grapple.verticesCache.begin(), grapple.verticesCache.end());
			}
			else
			{
				simpleGrapplesBuffers->verticesCache.insert(simpleGrapplesBuffers->verticesCache.end(), grapple.verticesCache.begin(), grapple.verticesCache.end());
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, simpleGrapplesBuffers->vertexBuffer);
		if (simpleGrapplesBuffers->vertexBufferAllocation < simpleGrapplesBuffers->verticesCache.size())
		{
			glBufferData(GL_ARRAY_BUFFER, simpleGrapplesBuffers->verticesCache.size() * sizeof(simpleGrapplesBuffers->verticesCache.front()),
				simpleGrapplesBuffers->verticesCache.data(), GL_STATIC_DRAW);
			simpleGrapplesBuffers->vertexBufferAllocation = simpleGrapplesBuffers->verticesCache.size();
		}
		else
		{
			glBufferSubData(GL_ARRAY_BUFFER, 0, simpleGrapplesBuffers->verticesCache.size() * sizeof(simpleGrapplesBuffers->verticesCache.front()),
				simpleGrapplesBuffers->verticesCache.data());
		}

		for (auto& [texture, texturedGrappleBuffers] : texturesToGrapplesBuffers)
		{
			glBindBuffer(GL_ARRAY_BUFFER, texturedGrappleBuffers.vertexBuffer);
			if (texturedGrappleBuffers.vertexBufferAllocation < texturedGrappleBuffers.verticesCache.size())
			{
				glBufferData(GL_ARRAY_BUFFER, texturedGrappleBuffers.verticesCache.size() * sizeof(texturedGrappleBuffers.verticesCache.front()),
					texturedGrappleBuffers.verticesCache.data(), GL_STATIC_DRAW);
				texturedGrappleBuffers.vertexBufferAllocation = texturedGrappleBuffers.verticesCache.size();
			}
			else
			{
				glBufferSubData(GL_ARRAY_BUFFER, 0, texturedGrappleBuffers.verticesCache.size() * sizeof(texturedGrappleBuffers.verticesCache.front()),
					texturedGrappleBuffers.verticesCache.data());
			}
		}
	}

	void Level::step()
	{
		updateDynamicWallsGraphics();
		updateGrapplesGraphics();
	}

	void Level::render() const
	{
		sceneCoordTexturedRender();
		texturedRender();
		basicRender();
	}

	void Level::sceneCoordTexturedRender() const
	{
		using namespace Globals::Components;

		glUseProgram_proxy(sceneCoordTexturedShadersProgram->program);
		sceneCoordTexturedShadersProgram->mvpUniform.setValue(mvp.getVP());

		for (const auto& [texture, texturedStaticWallBuffers] : texturesToStaticWallsBuffers)
		{
			const auto& textureComponent = textures[texture];
			const auto& textureDefComponent = texturesDef[texture];

			sceneCoordTexturedShadersProgram->texture1Uniform.setValue(texture);
			sceneCoordTexturedShadersProgram->textureTranslateUniform.setValue(
				{ textureDefComponent.translate.x, textureDefComponent.translate.y });
			sceneCoordTexturedShadersProgram->textureScaleUniform.setValue(
				{ (float)textureComponent.height / textureComponent.width * textureDefComponent.scale.x, textureDefComponent.scale.y });
			glBindVertexArray(texturedStaticWallBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, texturedStaticWallBuffers.verticesCache.size());
		}
	}

	void Level::texturedRender() const
	{
		using namespace Globals::Components;

		glUseProgram_proxy(texturedShadersProgram->program);
		texturedShadersProgram->mvpUniform.setValue(mvp.getVP());

		for (const auto& [texture, texturedDynamicWallBuffers] : texturesToDynamicWallsBuffers)
		{
			const auto& textureComponent = textures[texture];
			const auto& textureDefComponent = texturesDef[texture];

			texturedShadersProgram->texture1Uniform.setValue(texture);
			texturedShadersProgram->textureTranslateUniform.setValue({ textureDefComponent.translate.x, textureDefComponent.translate.y });
			texturedShadersProgram->textureScaleUniform.setValue(
				{ (float)textureComponent.height / textureComponent.width * textureDefComponent.scale.x, textureDefComponent.scale.y });
			glBindVertexArray(texturedDynamicWallBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, texturedDynamicWallBuffers.verticesCache.size());
		}

		for (const auto& [texture, texturedGrappleBuffers] : texturesToGrapplesBuffers)
		{
			const auto& textureComponent = textures[texture];
			const auto& textureDefComponent = texturesDef[texture];

			texturedShadersProgram->texture1Uniform.setValue(texture);
			texturedShadersProgram->textureTranslateUniform.setValue({ textureDefComponent.translate.x, textureDefComponent.translate.y });
			texturedShadersProgram->textureScaleUniform.setValue(
				{ (float)textureComponent.height / textureComponent.width * textureDefComponent.scale.x, textureDefComponent.scale.y });
			glBindVertexArray(texturedGrappleBuffers.vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, texturedGrappleBuffers.verticesCache.size());
		}
	}

	void Level::basicRender() const
	{
		using namespace Globals::Components;

		glUseProgram_proxy(basicShadersProgram->program);
		basicShadersProgram->mvpUniform.setValue(mvp.getVP());

		basicShadersProgram->colorUniform.setValue({ 0.5f, 0.5f, 0.5f, 1.0f });
		glBindVertexArray(simpleStaticWallsBuffers->vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, simpleStaticWallsBuffers->verticesCache.size());

		basicShadersProgram->colorUniform.setValue({ 0.5f, 0.5f, 0.5f, 1.0f });
		glBindVertexArray(simpleDynamicWallsBuffers->vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, simpleDynamicWallsBuffers->verticesCache.size());

		basicShadersProgram->colorUniform.setValue({ 0.0f, 0.5f, 0.0f, 1.0f });
		glBindVertexArray(simpleGrapplesBuffers->vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, simpleGrapplesBuffers->verticesCache.size());
	}

	Level::WallsBuffers::WallsBuffers(bool texCoord)
	{
		glCreateVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);
		if (texCoord)
		{
			texCoordBuffer = 0;
			glGenBuffers(1, &*texCoordBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, *texCoordBuffer);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(1);
		}
	}

	Level::WallsBuffers::~WallsBuffers()
	{
		glDeleteBuffers(1, &vertexBuffer);
		if (texCoordBuffer) glDeleteBuffers(1, &*texCoordBuffer);
		glDeleteVertexArrays(1, &vertexArray);
	}

	Level::GrapplesBuffers::GrapplesBuffers(bool texCoord)
	{
		glCreateVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);
		if (texCoord)
		{
			texCoordBuffer = 0;
			glGenBuffers(1, &*texCoordBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, *texCoordBuffer);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(1);
		}
	}

	Level::GrapplesBuffers::~GrapplesBuffers()
	{
		glDeleteBuffers(1, &vertexBuffer);
		if (texCoordBuffer) glDeleteBuffers(1, &*texCoordBuffer);
		glDeleteVertexArrays(1, &vertexArray);
	}
}
