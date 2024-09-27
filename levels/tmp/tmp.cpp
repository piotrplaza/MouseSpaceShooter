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
			auto& decorations = Globals::Components().dynamicDecorations().emplace(Tools::Shapes2D::CreateVerticesOfCircle(glm::linearRand(-screenInfo.getNormalizedWindowSize(), screenInfo.getNormalizedWindowSize()) * 9.0f, 1.0f, 20));
			//decorations.state = ComponentState::LastShot;
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
