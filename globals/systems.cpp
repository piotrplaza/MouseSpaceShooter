#include "systems.hpp"

#include "systems/stateController.hpp"
#include "systems/physics.hpp"
#include "systems/textures.hpp"
#include "systems/actors.hpp"
#include "systems/structures.hpp"
#include "systems/camera.hpp"
#include "systems/decorations.hpp"
#include "systems/temporaries.hpp"
#include "systems/cleaner.hpp"
#include "systems/deferredActions.hpp"
#include "systems/renderingController.hpp"
#include "systems/audio.hpp"

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

	Systems::Structures& SystemsHolder::structures()
	{
		return *structures_;
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

	Systems::Audio& SystemsHolder::audio()
	{
		return *audio_;
	}

	void InitializeSystems()
	{
		systemsHolder = std::make_unique<SystemsHolder>();
	}

	void TeardownSystems()
	{
		systemsHolder->actors().teardown();
		systemsHolder->physics().teardown();
	}

	void DestroySystems()
	{
		systemsHolder.reset();
	}

	SystemsHolder& Systems()
	{
		return *systemsHolder;
	}
}
