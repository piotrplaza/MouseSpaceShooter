#include "stateController.hpp"

#include <globals.hpp>
#include <components/player.hpp>
#include <components/grapple.hpp>

namespace Systems
{
	StateController::StateController() = default;
	StateController::~StateController() = default;

	void StateController::initializationFinalize() const
	{
		using namespace Globals::Components;

		for (auto& player : players)
			player.previousCenter = player.getCenter();

		for (auto& grapple : grapples)
			grapple.previousCenter = grapple.getCenter();
	}

	void StateController::frameSetup() const
	{
	}

	void StateController::frameTeardown() const
	{
		using namespace Globals::Components;

		for (auto& player : players)
			player.previousCenter = player.getCenter();

		for (auto& grapple : grapples)
			grapple.previousCenter = grapple.getCenter();
	}
}
