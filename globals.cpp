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
#include "components/texture.hpp"

#include "systems/physics.hpp"
#include "systems/player.hpp"
#include "systems/level.hpp"
#include "systems/camera.hpp"
#include "systems/textures.hpp"

namespace Components
{
	static ::Components::MouseState mouseState;
	static ::Components::ScreenInfo screenInfo;
	static ::Components::MVP mvp;
	static ::Components::Physics physics;
	static ::Components::Player player;
	static ::Components::Camera camera;

	static std::vector<::Components::Wall> staticWalls;
	static std::vector<::Components::Wall> dynamicWalls;
	static std::vector<::Components::Grapple> grapples;
	static std::vector<::Components::Connection> connections;
	static std::vector<::Components::Texture> textures;
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

		std::vector<::Components::Wall>& staticWalls = ::Components::staticWalls;
		std::vector<::Components::Wall>& dynamicWalls = ::Components::dynamicWalls;
		std::vector<::Components::Grapple>& grapples = ::Components::grapples;
		std::vector<::Components::Connection>& connections = ::Components::connections;
		std::vector<::Components::Texture>& textures = ::Components::textures;
	}

	namespace Systems
	{
		std::unique_ptr<::Systems::Physics> physics;
		std::unique_ptr<::Systems::Player> player;
		std::unique_ptr<::Systems::Level> level;
		std::unique_ptr<::Systems::Camera> camera;
		std::unique_ptr<::Systems::Textures> textures;

		void Initialize()
		{
			physics = std::make_unique<::Systems::Physics>();
			player = std::make_unique<::Systems::Player>();
			level = std::make_unique<::Systems::Level>();
			camera = std::make_unique<::Systems::Camera>();
			textures = std::make_unique<::Systems::Textures>();
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

		::Systems::Textures& AccessTextures()
		{
			return *textures;
		}
	}
}
