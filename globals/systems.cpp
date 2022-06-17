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
	static std::unique_ptr<SystemsHolder> systemsHolder;

	Systems::StateController& SystemsHolder::stateController()
	{
		return *stateController_;
	}

	Systems::Physics& SystemsHolder::physics()
	{
		return *physics_;
	}

	Systems::Textures& SystemsHolder::textures()
	{
		return *textures_;
	}

	Systems::Actors& SystemsHolder::actors()
	{
		return *actors_;
	}

	Systems::Walls& SystemsHolder::walls()
	{
		return *walls_;
	}

	Systems::Camera& SystemsHolder::camera()
	{
		return *camera_;
	}

	Systems::Decorations& SystemsHolder::decorations()
	{
		return *decorations_;
	}

	Systems::Temporaries& SystemsHolder::temporaries()
	{
		return *temporaries_;
	}

	Systems::Cleaner& SystemsHolder::cleaner()
	{
		return *cleaner_;
	}

	Systems::DeferredActions& SystemsHolder::deferredActions()
	{
		return *deferredActions_;
	}

	Systems::RenderingController& SystemsHolder::renderingController()
	{
		return *renderingController_;
	}

	void InitializeSystems()
	{
		systemsHolder = std::make_unique<SystemsHolder>();
	}

	SystemsHolder& Systems()
	{
		return *systemsHolder;
	}
}
