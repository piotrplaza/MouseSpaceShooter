#include <memory>

#include "components/mouseState.hpp"
#include "components/screenInfo.hpp"
#include "components/mvp.hpp"
#include "components/physics.hpp"
#include "components/player.hpp"

#include "systems/player.hpp"


namespace Components
{
	static ::Components::MouseState mouseState;
	static ::Components::ScreenInfo screenInfo;
	static ::Components::MVP mvp;
	static ::Components::Physics physics;
	static ::Components::Player player;
}

namespace Globals
{
	namespace Components
	{
		::Components::MouseState& mouseState = ::Components::mouseState;
		::Components::ScreenInfo& screenInfo = ::Components::screenInfo;
		::Components::MVP& mvp = ::Components::mvp;
		::Components::Physics& physics = ::Components::physics;
		::Components::Player& player = ::Components::player;
	}

	namespace Systems
	{
		std::unique_ptr<::Systems::Player> player;

		void Initialize()
		{
			player = std::make_unique<::Systems::Player>();
		}

		::Systems::Player& AccessPlayer()
		{
			return *player;
		}
	}
}
