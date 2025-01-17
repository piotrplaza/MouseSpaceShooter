#include <components/polyline.hpp>

#include <globals/components.hpp>

#include <tools/Shapes2D.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <unordered_set>

namespace GeneratedCode
{

inline void CreateStartingLine(ComponentId& startingLineId, glm::vec2& p1, glm::vec2& p2, float& startingPositionLineDistance)
{
	const glm::vec2 cp1 = { -1.55, 7.45 };
	const glm::vec2 cp2 = { -1.6, -13.55 };
	auto& startingLine = Globals::Components().staticPolylines().emplace(std::vector<glm::vec2>{ cp1, cp2 },
		Tools::BodyParams().sensor(true)); 
	startingLine.colorF = []() { return glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); };

	startingLineId = startingLine.getComponentId();
	p1 = cp1;
	p2 = cp2;
	startingPositionLineDistance = 1;
}

inline void CreateDeadlySplines(const Tools::PlayersHandler& playersHandler, std::unordered_set<ComponentId>& deadlySplines)
{
}

}
