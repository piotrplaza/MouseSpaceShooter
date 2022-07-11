#pragma once

#include <memory>

namespace Systems
{
	class StateController;
	class Physics;
	class Textures;
	class Actors;
	class Structures;
	class Camera;
	class Decorations;
	class Temporaries;
	class Cleaner;
	class DeferredActions;
	class RenderingController;
}

namespace Globals
{
	class SystemsHolder
	{
	public:
		Systems::StateController& stateController();
		Systems::Physics& physics();
		Systems::Textures& textures();
		Systems::Actors& actors();
		Systems::Structures& walls();
		Systems::Camera& camera();
		Systems::Decorations& decorations();
		Systems::Temporaries& temporaries();
		Systems::Cleaner& cleaner();
		Systems::DeferredActions& deferredActions();
		Systems::RenderingController& renderingController();

	private:
		std::unique_ptr<Systems::StateController> stateController_ = std::make_unique<Systems::StateController>();
		std::unique_ptr<Systems::Physics> physics_ = std::make_unique<Systems::Physics>();
		std::unique_ptr<Systems::Textures> textures_ = std::make_unique<Systems::Textures>();
		std::unique_ptr<Systems::Actors> actors_ = std::make_unique<Systems::Actors>();
		std::unique_ptr<Systems::Structures> structures_ = std::make_unique<Systems::Structures>();
		std::unique_ptr<Systems::Camera> camera_ = std::make_unique<Systems::Camera>();
		std::unique_ptr<Systems::Decorations> decorations_ = std::make_unique<Systems::Decorations>();
		std::unique_ptr<Systems::Temporaries> temporaries_ = std::make_unique<Systems::Temporaries>();
		std::unique_ptr<Systems::Cleaner> cleaner_ = std::make_unique<Systems::Cleaner>();
		std::unique_ptr<Systems::DeferredActions> deferredActions_ = std::make_unique<Systems::DeferredActions>();
		std::unique_ptr<Systems::RenderingController> renderingController_ = std::make_unique<Systems::RenderingController>();
	};

	void InitializeSystems();

	SystemsHolder& Systems();
}
