#include <memory>
#include <vector>

#include "components/mouseState.hpp"
#include "components/screenInfo.hpp"
#include "components/mvp.hpp"
#include "components/physics.hpp"
#include "components/player.hpp"
#include "components/wall.hpp"
#include "components/grapple.hpp"
#include "components/connection.hpp"
#include "components/camera.hpp"

#include "systems/physics.hpp"
#include "systems/player.hpp"
#include "systems/level.hpp"
#include "systems/camera.hpp"

namespace Components
{
	static ::Components::MouseState mouseState;
	static ::Components::ScreenInfo screenInfo;
	static ::Components::MVP mvp;
	static ::Components::Physics physics;
	static ::Components::Player player;
	static ::Components::Camera camera;

	static std::vector<::Components::Wall> walls;
	static std::vector<::Components::Grapple> grapples;
	static std::vector<::Components::Connection> connections;
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
		::Components::Camera& camera = ::Components::camera;

		std::vector<::Components::Wall>& walls = ::Components::walls;
		std::vector<::Components::Grapple>& grapples = ::Components::grapples;
		std::vector<::Components::Connection>& connections = ::Components::connections;
	}

	namespace Systems
	{
		std::unique_ptr<::Systems::Physics> physics;
		std::unique_ptr<::Systems::Player> player;
		std::unique_ptr<::Systems::Level> level;
		std::unique_ptr<::Systems::Camera> camera;

		void Initialize()
		{
			physics = std::make_unique<::Systems::Physics>();
			player = std::make_unique<::Systems::Player>();
			level = std::make_unique<::Systems::Level>();
			camera = std::make_unique<::Systems::Camera>();
		}

		::Systems::Physics& AccessPhysics()
		{
			return *physics;
		}

		::Systems::Player& AccessPlayer()
		{
			return *player;
		}

		::Systems::Level& AccessLevel()
		{
			return *level;
		}

		::Systems::Camera& AccessCamera()
		{
			return *camera;
		}
	}
}
