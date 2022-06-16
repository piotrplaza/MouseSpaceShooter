#include "systems.hpp"

#include "systems/stateController.hpp"
#include "systems/physics.hpp"
#include "systems/textures.hpp"
#include "systems/actors.hpp"
#include "systems/walls.hpp"
#include "systems/camera.hpp"
#include "systems/decorations.hpp"
#include "systems/temporaries.hpp"
#include "systems/cleaner.hpp"
#include "systems/deferredActions.hpp"
#include "systems/renderingController.hpp"

namespace Globals
{
	static std::unique_ptr<class Systems> systems;

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

	::Systems::Actors& Systems::actors()
	{
		return *actors_;
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

	void InitializeSystems()
	{
		systems = std::make_unique<class Systems>();
	}

	class Systems& Systems()
	{
		return *systems;
	}
}
