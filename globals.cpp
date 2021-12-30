#include "globals.hpp"

#include "ogl/shaders/basic.hpp"
#include "ogl/shaders/colored.hpp"
#include "ogl/shaders/julia.hpp"
#include "ogl/shaders/particles.hpp"
#include "ogl/shaders/sceneCoordTextured.hpp"
#include "ogl/shaders/textured.hpp"
#include "ogl/shaders/texturedColorThreshold.hpp"

#include "components/mouseState.hpp"
#include "components/screenInfo.hpp"
#include "components/mvp.hpp"
#include "components/physics.hpp"
#include "components/texture.hpp"
#include "components/animationTexture.hpp"
#include "components/blendingTexture.hpp"
#include "components/renderingSetup.hpp"
#include "components/player.hpp"
#include "components/wall.hpp"
#include "components/grapple.hpp"
#include "components/camera.hpp"
#include "components/decoration.hpp"
#include "components/graphicsSettings.hpp"
#include "components/missile.hpp"
#include "components/collisionHandler.hpp"
#include "components/shockwave.hpp"
#include "components/light.hpp"
#include "components/framebuffers.hpp"
#include "components/functor.hpp"
#include "components/mainFramebufferRenderer.hpp"

#include "systems/stateController.hpp"
#include "systems/physics.hpp"
#include "systems/textures.hpp"
#include "systems/players.hpp"
#include "systems/walls.hpp"
#include "systems/camera.hpp"
#include "systems/decorations.hpp"
#include "systems/temporaries.hpp"
#include "systems/cleaner.hpp"
#include "systems/deferredActions.hpp"
#include "systems/renderingController.hpp"

#include <memory>
#include <vector>
#include <unordered_map>

namespace Globals
{
	static std::unique_ptr<class Shaders> shaders;
	static std::unique_ptr<::ComponentIdGenerator> componentIdGenerator;
	static std::unique_ptr<class Systems> systems;
	static std::unique_ptr<class Components> components;


	::Shaders::Programs::Basic& Shaders::basic()
	{
		return *basic_;
	}

	::Shaders::Programs::Colored& Shaders::colored()
	{
		return *colored_;
	}

	::Shaders::Programs::Julia& Shaders::julia()
	{
		return *julia_;
	}

	::Shaders::Programs::Particles& Shaders::particles()
	{
		return *particles_;
	}

	::Shaders::Programs::SceneCoordTextured& Shaders::sceneCoordTextured()
	{
		return *sceneCoordTextured_;
	}

	::Shaders::Programs::Textured& Shaders::textured()
	{
		return *textured_;
	}

	::Shaders::Programs::TexturedColorThreshold& Shaders::texturedColorThreshold()
	{
		return *texturedColorThreshold_;
	}


	Components::Components()
	{
		textures_.emplace_back(fakeComponent);
		animationTextures_.emplace_back(fakeComponent);
		blendingTextures_.emplace_back(fakeComponent);
		renderingSetups_.emplace_back(fakeComponent);
		players_.emplace_back(fakeComponent);
		staticWalls_.emplace_back(fakeComponent);
		dynamicWalls_.emplace_back(fakeComponent);
		grapples_.emplace_back(fakeComponent);
		backgroundDecorations_.emplace_back(fakeComponent);
		farMidgroundDecorations_.emplace_back(fakeComponent);
		midgroundDecorations_.emplace_back(fakeComponent);
		nearMidgroundDecorations_.emplace_back(fakeComponent);
		foregroundDecorations_.emplace_back(fakeComponent);
	}

	::Components::MouseState& Components::mouseState()
	{
		return *mouseState_;
	}

	::Components::ScreenInfo& Components::screenInfo()
	{
		return *screenInfo_;
	}

	::Components::MVP& Components::mvp()
	{
		return *mvp_;
	}

	::Components::Physics& Components::physics()
	{
		return *physics_;
	}

	::Components::Camera& Components::camera()
	{
		return *camera_;
	}

	::Components::GraphicsSettings& Components::graphicsSettings()
	{
		return *graphicsSettings_;
	}

	::Components::Framebuffers& Components::framebuffers()
	{
		return *framebuffers_;
	}

	::Components::MainFramebufferRenderer& Components::mainFramebufferRenderer()
	{
		return *mainFramebufferRenderer_;
	}


	std::vector<::Components::Texture>& Components::textures()
	{
		return textures_;
	}

	std::vector<::Components::AnimationTexture>& Components::animationTextures()
	{
		return animationTextures_;
	}

	std::vector<::Components::BlendingTexture>& Components::blendingTextures()
	{
		return blendingTextures_;
	}

	std::vector<::Components::RenderingSetup>& Components::renderingSetups()
	{
		return renderingSetups_;
	}

	std::vector<::Components::Player>& Components::players()
	{
		return players_;
	}

	std::vector<::Components::Wall>& Components::staticWalls()
	{
		return staticWalls_;
	}

	std::vector<::Components::Wall>& Components::dynamicWalls()
	{
		return dynamicWalls_;
	}

	std::vector<::Components::Grapple>& Components::grapples()
	{
		return grapples_;
	}

	std::vector<::Components::Decoration>& Components::backgroundDecorations()
	{
		return backgroundDecorations_;
	}

	std::unordered_map<::ComponentId, ::Components::Decoration>& Components::temporaryBackgroundDecorations()
	{
		return temporaryBackgroundDecorations_;
	}

	std::vector<::Components::Decoration>& Components::farMidgroundDecorations()
	{
		return farMidgroundDecorations_;
	}

	std::unordered_map<::ComponentId, ::Components::Decoration>& Components::temporaryFarMidgroundDecorations()
	{
		return temporaryFarMidgroundDecorations_;
	}

	std::vector<::Components::Decoration>& Components::midgroundDecorations()
	{
		return midgroundDecorations_;
	}

	std::unordered_map<::ComponentId, ::Components::Decoration>& Components::temporaryMidgroundDecorations()
	{
		return temporaryMidgroundDecorations_;
	}

	std::vector<::Components::Decoration>& Components::nearMidgroundDecorations()
	{
		return nearMidgroundDecorations_;
	}

	std::unordered_map<::ComponentId, ::Components::Decoration>& Components::temporaryNearMidgroundDecorations()
	{
		return temporaryNearMidgroundDecorations_;
	}

	std::vector<::Components::Decoration>& Components::foregroundDecorations()
	{
		return foregroundDecorations_;
	}

	std::unordered_map<::ComponentId, ::Components::Decoration>& Components::temporaryForegroundDecorations()
	{
		return temporaryForegroundDecorations_;
	}

	std::unordered_map<::ComponentId, ::Components::Missile>& Components::missiles()
	{
		return missiles_;
	}

	std::unordered_map<::ComponentId, ::Components::CollisionHandler>& Components::beginCollisionHandlers()
	{
		return beginCollisionHandlers_;
	}

	std::unordered_map<::ComponentId, ::Components::CollisionHandler>& Components::endCollisionHandlers()
	{
		return endCollisionHandlers_;
	}

	std::unordered_map<::ComponentId, ::Components::Shockwave>& Components::shockwaves()
	{
		return shockwaves_;
	}

	std::unordered_map<::ComponentId, ::Components::Light>& Components::lights()
	{
		return lights_;
	}

	std::unordered_map<::ComponentId, ::Components::Functor>& Components::frameSetups()
	{
		return frameSetups_;
	}

	std::unordered_map<::ComponentId, ::Components::Functor>& Components::frameTeardowns()
	{
		return frameTeardowns_;
	}


	::Systems::StateController& Systems::stateController()
	{
		return *stateController_;
	}

	::Systems::Physics& Systems::physics()
	{
		return *physics_;
	}

	::Systems::Textures& Systems::textures()
	{
		return *textures_;
	}

	::Systems::Players& Systems::players()
	{
		return *players_;
	}

	::Systems::Walls& Systems::walls()
	{
		return *walls_;
	}

	::Systems::Camera& Systems::camera()
	{
		return *camera_;
	}

	::Systems::Decorations& Systems::decorations()
	{
		return *decorations_;
	}

	::Systems::Temporaries& Systems::temporaries()
	{
		return *temporaries_;
	}

	::Systems::Cleaner& Systems::cleaner()
	{
		return *cleaner_;
	}

	::Systems::DeferredActions& Systems::deferredActions()
	{
		return *deferredActions_;
	}

	::Systems::RenderingController& Systems::renderingController()
	{
		return *renderingController_;
	}


	void InitializeShaders()
	{
		shaders = std::make_unique<class Shaders>();
	}

	void InitializeComponents()
	{
		componentIdGenerator = std::make_unique<::ComponentIdGenerator>();
		components = std::make_unique<class Components>();
	}

	void InitializeSystems()
	{
		systems = std::make_unique<class Systems>();
	}


	class Shaders& Shaders()
	{
		return *shaders;
	}

	::ComponentIdGenerator& ComponentIdGenerator()
	{
		return *componentIdGenerator;
	}

	class Components& Components()
	{
		return *components;
	}

	class Systems& Systems()
	{
		return *systems;
	}
}
