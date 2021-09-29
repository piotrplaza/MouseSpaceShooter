#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "componentId.hpp"

namespace Shaders::Programs
{
	struct Basic;
	struct Colored;
	struct Julia;
	struct Particles;
	struct SceneCoordTextured;
	struct Textured;
	struct TexturedColorThreshold;
}

namespace Components
{
	struct MouseState;
	struct ScreenInfo;
	struct MVP;
	struct Physics;
	struct Texture;
	struct Player;
	struct Wall;
	struct Grapple;
	struct Camera;
	struct Decoration;
	struct GraphicsSettings;
	struct Missile;
	struct CollisionHandler;
	struct Shockwave;
	struct Light;
	struct Framebuffers;
	struct Functor;
	struct MainFramebufferRenderer;
}

namespace Systems
{
	class StateController;
	class Physics;
	class Textures;
	class Players;
	class Walls;
	class Camera;
	class Decorations;
	class Temporaries;
	class Cleaner;
	class DeferredActions;
	class RenderingController;
}

namespace Globals
{
	class Shaders
	{
	public:
		::Shaders::Programs::Basic& basic();
		::Shaders::Programs::Colored& colored();
		::Shaders::Programs::Julia& julia();
		::Shaders::Programs::Particles& particles();
		::Shaders::Programs::SceneCoordTextured& sceneCoordTextured();
		::Shaders::Programs::Textured& textured();
		::Shaders::Programs::TexturedColorThreshold& texturedColorThreshold();

	private:
		std::unique_ptr<::Shaders::Programs::Basic> basic_ = std::make_unique<::Shaders::Programs::Basic>();
		std::unique_ptr<::Shaders::Programs::Colored> colored_ = std::make_unique<::Shaders::Programs::Colored>();
		std::unique_ptr<::Shaders::Programs::Julia> julia_ = std::make_unique<::Shaders::Programs::Julia>();
		std::unique_ptr<::Shaders::Programs::Particles> particles_ = std::make_unique<::Shaders::Programs::Particles>();
		std::unique_ptr<::Shaders::Programs::SceneCoordTextured> sceneCoordTextured_ = std::make_unique<::Shaders::Programs::SceneCoordTextured>();
		std::unique_ptr<::Shaders::Programs::Textured> textured_ = std::make_unique<::Shaders::Programs::Textured>();
		std::unique_ptr<::Shaders::Programs::TexturedColorThreshold> texturedColorThreshold_ = std::make_unique<::Shaders::Programs::TexturedColorThreshold>();
	};

	class Components
	{
	public:
		::Components::MouseState& mouseState();
		::Components::ScreenInfo& screenInfo();
		::Components::MVP& mvp();
		::Components::Physics& physics();
		::Components::Camera& camera();
		::Components::GraphicsSettings& graphicsSettings();
		::Components::Framebuffers& framebuffers();
		::Components::MainFramebufferRenderer& mainFramebufferRenderer();

		std::vector<::Components::Texture>& textures();
		std::vector<::Components::Player>& players();
		std::vector<::Components::Wall>& staticWalls();
		std::vector<::Components::Wall>& dynamicWalls();
		std::vector<::Components::Grapple>& grapples();
		std::vector<::Components::Decoration>& backgroundDecorations();
		std::unordered_map<::ComponentId, ::Components::Decoration>& temporaryBackgroundDecorations();
		std::vector<::Components::Decoration>& farMidgroundDecorations();
		std::unordered_map<::ComponentId, ::Components::Decoration>& temporaryFarMidgroundDecorations();
		std::vector<::Components::Decoration>& midgroundDecorations();
		std::unordered_map<::ComponentId, ::Components::Decoration>& temporaryMidgroundDecorations();
		std::vector<::Components::Decoration>& nearMidgroundDecorations();
		std::unordered_map<::ComponentId, ::Components::Decoration>& temporaryNearMidgroundDecorations();
		std::vector<::Components::Decoration>& foregroundDecorations();
		std::unordered_map<::ComponentId, ::Components::Decoration>& temporaryForegroundDecorations();
		std::unordered_map<::ComponentId, ::Components::Missile>& missiles();
		std::unordered_map<::ComponentId, ::Components::CollisionHandler>& beginCollisionHandlers();
		std::unordered_map<::ComponentId, ::Components::CollisionHandler>& endCollisionHandlers();
		std::unordered_map<::ComponentId, ::Components::Shockwave>& shockwaves();
		std::unordered_map<::ComponentId, ::Components::Light>& lights();
		std::unordered_map<::ComponentId, ::Components::Functor>& frameSetups();
		std::unordered_map<::ComponentId, ::Components::Functor>& frameTeardowns();

	private:
		std::unique_ptr<::Components::MouseState> mouseState_ = std::make_unique<::Components::MouseState>();
		std::unique_ptr<::Components::ScreenInfo> screenInfo_ = std::make_unique<::Components::ScreenInfo>();
		std::unique_ptr<::Components::MVP> mvp_ = std::make_unique<::Components::MVP>();
		std::unique_ptr<::Components::Physics> physics_ = std::make_unique<::Components::Physics>();
		std::unique_ptr<::Components::Camera> camera_ = std::make_unique<::Components::Camera>();
		std::unique_ptr<::Components::GraphicsSettings> graphicsSettings_ = std::make_unique<::Components::GraphicsSettings>();
		std::unique_ptr<::Components::Framebuffers> framebuffers_ = std::make_unique<::Components::Framebuffers>();
		std::unique_ptr<::Components::MainFramebufferRenderer> mainFramebufferRenderer_ = std::make_unique<::Components::MainFramebufferRenderer>();

		std::vector<::Components::Texture> textures_;
		std::vector<::Components::Player> players_;
		std::vector<::Components::Wall> staticWalls_;
		std::vector<::Components::Wall> dynamicWalls_;
		std::vector<::Components::Grapple> grapples_;
		std::vector<::Components::Decoration> backgroundDecorations_;
		std::unordered_map<::ComponentId, ::Components::Decoration> temporaryBackgroundDecorations_;
		std::vector<::Components::Decoration> farMidgroundDecorations_;
		std::unordered_map<::ComponentId, ::Components::Decoration> temporaryFarMidgroundDecorations_;
		std::vector<::Components::Decoration> midgroundDecorations_;
		std::unordered_map<::ComponentId, ::Components::Decoration> temporaryMidgroundDecorations_;
		std::vector<::Components::Decoration> nearMidgroundDecorations_;
		std::unordered_map<::ComponentId, ::Components::Decoration> temporaryNearMidgroundDecorations_;
		std::vector<::Components::Decoration> foregroundDecorations_;
		std::unordered_map<::ComponentId, ::Components::Decoration> temporaryForegroundDecorations_;
		std::unordered_map<::ComponentId, ::Components::Missile> missiles_;
		std::unordered_map<::ComponentId, ::Components::CollisionHandler> beginCollisionHandlers_;
		std::unordered_map<::ComponentId, ::Components::CollisionHandler> endCollisionHandlers_;
		std::unordered_map<::ComponentId, ::Components::Shockwave> shockwaves_;
		std::unordered_map<::ComponentId, ::Components::Light> lights_;
		std::unordered_map<::ComponentId, ::Components::Functor> frameSetups_;
		std::unordered_map<::ComponentId, ::Components::Functor> frameTeardowns_;
	};

	class Systems
	{
	public:
		::Systems::StateController& stateController();
		::Systems::Physics& physics();
		::Systems::Textures& textures();
		::Systems::Players& players();
		::Systems::Walls& walls();
		::Systems::Camera& camera();
		::Systems::Decorations& decorations();
		::Systems::Temporaries& temporaries();
		::Systems::Cleaner& cleaner();
		::Systems::DeferredActions& deferredActions();
		::Systems::RenderingController& renderingController();

	private:
		std::unique_ptr<::Systems::StateController> stateController_ = std::make_unique<::Systems::StateController>();
		std::unique_ptr<::Systems::Physics> physics_ = std::make_unique<::Systems::Physics>();
		std::unique_ptr<::Systems::Textures> textures_ = std::make_unique<::Systems::Textures>();
		std::unique_ptr<::Systems::Players> players_ = std::make_unique<::Systems::Players>();
		std::unique_ptr<::Systems::Walls> walls_ = std::make_unique<::Systems::Walls>();
		std::unique_ptr<::Systems::Camera> camera_ = std::make_unique<::Systems::Camera>();
		std::unique_ptr<::Systems::Decorations> decorations_ = std::make_unique<::Systems::Decorations>();
		std::unique_ptr<::Systems::Temporaries> temporaries_ = std::make_unique<::Systems::Temporaries>();
		std::unique_ptr<::Systems::Cleaner> cleaner_ = std::make_unique<::Systems::Cleaner>();
		std::unique_ptr<::Systems::DeferredActions> deferredActions_ = std::make_unique<::Systems::DeferredActions>();
		std::unique_ptr<::Systems::RenderingController> renderingController_ = std::make_unique<::Systems::RenderingController>();
	};

	void InitializeShaders();
	void InitializeComponents();
	void InitializeSystems();

	class Shaders& Shaders();
	::ComponentIdGenerator& ComponentIdGenerator();
	class Components& Components();
	class Systems& Systems();
}
