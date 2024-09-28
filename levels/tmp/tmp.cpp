#include "tmp.hpp"

#include <components/decoration.hpp>
#include <components/screenInfo.hpp>
#include <components/physics.hpp>
#include <globals/components.hpp>
#include <tools/Shapes2D.hpp>

#include <glm/gtc/random.hpp>

namespace Levels
{
	class Tmp::Impl
	{
	public:
		void setup()
		{
		}

		void step()
		{
			const auto& screenInfo = Globals::Components().screenInfo();
			const auto pos = glm::linearRand(-screenInfo.getNormalizedWindowSize(), screenInfo.getNormalizedWindowSize()) * 9.0f;
			auto& decorations = Globals::Components().dynamicDecorations().emplace(Tools::Shapes2D::CreateVerticesOfCircle(pos, 1.0f, 20));
			decorations.subsequence.emplace_back(Tools::Shapes2D::CreateVerticesOfCircle(pos, 0.2f, 20));
			decorations.subsequence.back().colorF = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			decorations.state = ComponentState::LastShot;
		}
	};

	Tmp::Tmp():
		impl(std::make_unique<Impl>())
	{
		impl->setup();
	}

	Tmp::~Tmp() = default;

	void Tmp::step()
	{
		impl->step();
	}
}
