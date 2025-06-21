#include <components/polyline.hpp>
#include <components/decoration.hpp>
#include <components/texture.hpp>

#include <globals/components.hpp>

#include <tools/Shapes2D.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <unordered_set>

namespace GeneratedCode
{

inline glm::vec4 GetBackgroundColor()
{
	return {0, 0.05, 0.2, 1.0f};
}

inline void CreateBackground(ComponentId& backgroundTextureId, ComponentId& backgroundDecorationId)
{
	backgroundTextureId = Globals::Components().staticTextures().size();
	Globals::Components().staticTextures().emplace("textures/racing/space track 1.jpg");
	auto& background = Globals::Components().staticDecorations().emplace(Tools::Shapes2D::CreatePositionsOfRectangle({}, { 0.5f * 1, 0.5f }));
	background.texCoord = Tools::Shapes2D::CreateTexCoordOfRectangle();
	background.modelMatrixF = glm::scale(glm::translate(glm::mat4{ 1.0f }, glm::vec3(glm::vec2(0, 0), 0.0f)), 	glm::vec3(glm::vec2(200, 200), 1.0f));
	background.renderLayer = RenderLayer::Background;
	background.texture = CM::Texture(backgroundTextureId, true);
	backgroundDecorationId = background.getComponentId();
}

inline void CreateStartingLine(ComponentId& startingLineId, glm::vec2& p1, glm::vec2& p2, float& startingPositionLineDistance)
{
}

inline void CreateDeadlySplines(const Tools::PlayersHandler& playersHandler, std::unordered_set<ComponentId>& deadlySplines)
{
}

inline void CreateGrapples()
{
}

}
