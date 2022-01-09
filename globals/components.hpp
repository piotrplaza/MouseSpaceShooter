#pragma once

#include <components/componentId.hpp>

#include <memory>
#include <vector>
#include <unordered_map>

namespace Components
{
	struct MouseState;
	struct ScreenInfo;
	struct MVP;
	struct Physics;
	struct Texture;
	struct AnimationTexture;
	struct BlendingTexture;
	struct RenderingSetup;
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

namespace Globals
{
	class Components
	{
	public:
		Components();

		::Components::MouseState& mouseState();
		::Components::ScreenInfo& screenInfo();
		::Components::MVP& mvp();
		::Components::Physics& physics();
		::Components::Camera& camera();
		::Components::GraphicsSettings& graphicsSettings();
		::Components::Framebuffers& framebuffers();
		::Components::MainFramebufferRenderer& mainFramebufferRenderer();

		std::vector<::Components::Texture>& textures();
		std::vector<::Components::AnimationTexture>& animationTextures();
		std::vector<::Components::BlendingTexture>& blendingTextures();
		std::vector<::Components::RenderingSetup>& renderingSetups();
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
		std::vector<::Components::AnimationTexture> animationTextures_;
		std::vector<::Components::BlendingTexture> blendingTextures_;
		std::vector<::Components::RenderingSetup> renderingSetups_;
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

	void InitializeComponents();

	::ComponentIdGenerator& ComponentIdGenerator();
	class Components& Components();

	template <typename Component, typename F>
	inline void ForEach(std::vector<Component>& components, F f)
	{
		for (auto it = std::next(components.begin()); it != components.end(); ++it)
		{
			f(*it);
		}
	}

	template <typename Component, typename F>
	inline void ForEach(const std::vector<Component>& components, F f)
	{
		for (auto it = std::next(components.begin()); it != components.end(); ++it)
		{
			f(*it);
		}
	}
}
