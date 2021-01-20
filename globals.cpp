#include <memory>
#include <vector>
#include <unordered_map>

#include "componentId.hpp"

#include "components/mouseState.hpp"
#include "components/screenInfo.hpp"
#include "components/mvp.hpp"
#include "components/physics.hpp"
#include "components/textureDef.hpp"
#include "components/texture.hpp"
#include "components/player.hpp"
#include "components/wall.hpp"
#include "components/grapple.hpp"
#include "components/camera.hpp"
#include "components/decoration.hpp"
#include "components/graphicsSettings.hpp"
#include "components/missile.hpp"
#include "components/collisionHandler.hpp"

#include "systems/stateController.hpp"
#include "systems/physics.hpp"
#include "systems/textures.hpp"
#include "systems/players.hpp"
#include "systems/walls.hpp"
#include "systems/camera.hpp"
#include "systems/decorations.hpp"
#include "systems/temporaries.hpp"
#include "systems/cleaner.hpp"

ComponentIdGenerator ComponentIdGenerator::instance_;

namespace Components
{
	static ::Components::MouseState mouseState;
	static ::Components::ScreenInfo screenInfo;
	static ::Components::MVP mvp;
	static ::Components::Physics physics;
	static ::Components::Camera camera;
	static ::Components::GraphicsSettings graphicsSettings;

	static std::vector<::Components::TextureDef> texturesDef;
	static std::vector<::Components::Texture> textures;
	static std::vector<::Components::Player> players;
	static std::vector<::Components::Wall> staticWalls;
	static std::vector<::Components::Wall> dynamicWalls;
	static std::vector<::Components::Grapple> grapples;
	static std::vector<::Components::Decoration> backgroundDecorations;
	static std::unordered_map<::ComponentId, ::Components::Decoration> temporaryBackgroundDecorations;
	static std::vector<::Components::Decoration> midgroundDecorations;
	static std::unordered_map<::ComponentId, ::Components::Decoration> temporaryMidgroundDecorations;
	static std::vector<::Components::Decoration> foregroundDecorations;
	static std::unordered_map<::ComponentId, ::Components::Decoration> temporaryForegroundDecorations;
	static std::unordered_map<::ComponentId, ::Components::Missile> missiles;
	static std::unordered_map<::ComponentId, ::Components::CollisionHandler> beginCollisionHandlers;
	static std::unordered_map<::ComponentId, ::Components::CollisionHandler> endCollisionHandlers;
}

namespace Globals
{
	namespace Components
	{
		::Components::MouseState& mouseState = ::Components::mouseState;
		::Components::ScreenInfo& screenInfo = ::Components::screenInfo;
		::Components::MVP& mvp = ::Components::mvp;
		::Components::Physics& physics = ::Components::physics;
		::Components::Camera& camera = ::Components::camera;
		::Components::GraphicsSettings& graphicsSettings = ::Components::graphicsSettings;

		std::vector<::Components::TextureDef>& texturesDef = ::Components::texturesDef;
		std::vector<::Components::Texture>& textures = ::Components::textures;
		std::vector<::Components::Player>& players = ::Components::players;
		std::vector<::Components::Wall>& staticWalls = ::Components::staticWalls;
		std::vector<::Components::Wall>& dynamicWalls = ::Components::dynamicWalls;
		std::vector<::Components::Grapple>& grapples = ::Components::grapples;
		std::vector<::Components::Decoration>& backgroundDecorations = ::Components::backgroundDecorations;
		std::unordered_map<::ComponentId, ::Components::Decoration>& temporaryBackgroundDecorations = ::Components::temporaryBackgroundDecorations;
		std::vector<::Components::Decoration>& midgroundDecorations = ::Components::midgroundDecorations;
		std::unordered_map<::ComponentId, ::Components::Decoration>& temporaryMidgroundDecorations = ::Components::temporaryMidgroundDecorations;
		std::vector<::Components::Decoration>& foregroundDecorations = ::Components::foregroundDecorations;
		std::unordered_map<::ComponentId, ::Components::Decoration>& temporaryForegroundDecorations = ::Components::temporaryForegroundDecorations;
		std::unordered_map<::ComponentId, ::Components::Missile>& missiles = ::Components::missiles;
		std::unordered_map<::ComponentId, ::Components::CollisionHandler>& beginCollisionHandlers = ::Components::beginCollisionHandlers;
		std::unordered_map<::ComponentId, ::Components::CollisionHandler>& endCollisionHandlers = ::Components::endCollisionHandlers;
	}

	namespace Systems
	{
		std::unique_ptr<::Systems::StateController> stateController;
		std::unique_ptr<::Systems::Physics> physics;
		std::unique_ptr<::Systems::Textures> textures;
		std::unique_ptr<::Systems::Players> players;
		std::unique_ptr<::Systems::Walls> walls;
		std::unique_ptr<::Systems::Camera> camera;
		std::unique_ptr<::Systems::Decorations> decorations;
		std::unique_ptr<::Systems::Temporaries> temporaries;
		std::unique_ptr<::Systems::Cleaner> cleaner;

		void Initialize()
		{
			stateController = std::make_unique<::Systems::StateController>();
			physics = std::make_unique<::Systems::Physics>();
			textures = std::make_unique<::Systems::Textures>();
			players = std::make_unique<::Systems::Players>();
			walls = std::make_unique<::Systems::Walls>();
			camera = std::make_unique<::Systems::Camera>();
			decorations = std::make_unique<::Systems::Decorations>();
			temporaries = std::make_unique<::Systems::Temporaries>();
			cleaner = std::make_unique<::Systems::Cleaner>();
		}

		::Systems::StateController& AccessStateController()
		{
			return *stateController;
		}

		::Systems::Physics& AccessPhysics()
		{
			return *physics;
		}

		::Systems::Textures& AccessTextures()
		{
			return *textures;
		}

		::Systems::Players& AccessPlayers()
		{
			return *players;
		}

		::Systems::Walls& AccessWalls()
		{
			return *walls;
		}

		::Systems::Camera& AccessCamera()
		{
			return *camera;
		}

		::Systems::Decorations& AccessDecorations()
		{
			return *decorations;
		}

		::Systems::Temporaries& AccessTemporaries()
		{
			return *temporaries;
		}

		::Systems::Cleaner& AccessCleaner()
		{
			return *cleaner;
		}
	}
}
