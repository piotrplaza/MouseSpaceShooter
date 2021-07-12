#include "stateController.hpp"

#include "tools/utility.hpp"

#include <globals.hpp>

#include <components/player.hpp>
#include <components/grapple.hpp>
#include <components/screenInfo.hpp>
#include <components/mouseState.hpp>
#include <components/graphicsSettings.hpp>
#include <components/lowResBuffers.hpp>
#include <components/texture.hpp>

namespace
{
	constexpr int lowResDivisor = 2;
}

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

		createLowResFramebuffer();
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

	void StateController::changeWindowSize(glm::ivec2 size) const
	{
		using namespace Globals::Components;

		screenInfo.windowSize = size;
		screenInfo.windowCenterInScreenSpace = { screenInfo.windowLocation + screenInfo.windowSize / 2 };

		glBindFramebuffer(GL_FRAMEBUFFER, lowResBuffers.fbo);
		glBindTexture(GL_TEXTURE_2D, lowResBuffers.textureObject);
		lowResBuffers.size = size / lowResDivisor;
		textures[lowResBuffers.textureUnit - GL_TEXTURE0].size = lowResBuffers.size;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, lowResBuffers.size.x, lowResBuffers.size.y, 0, GL_RGBA, GL_FLOAT, nullptr);
		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}

	void StateController::changeWindowLocation(glm::ivec2 location) const
	{
		using namespace Globals::Components;

		screenInfo.windowLocation = location;
		screenInfo.windowCenterInScreenSpace = { location + screenInfo.windowSize / 2 };
	}

	void StateController::resetMousePosition() const
	{
		using namespace Globals::Components;

		Tools::SetMousePos(screenInfo.windowCenterInScreenSpace);
		mouseState.position = screenInfo.windowCenterInScreenSpace;
	}

	void StateController::handleMousePosition() const
	{
		using namespace Globals::Components;

		const auto prevPosition = mouseState.position;
		mouseState.position = Tools::GetMousePos();
		mouseState.delta = mouseState.position - prevPosition;

		resetMousePosition();
	}

	void StateController::handleKeyboard(bool const* const keys) const
	{
	}

	void StateController::createLowResFramebuffer() const
	{
		using namespace Globals::Components;

		glGenFramebuffers(1, &lowResBuffers.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, lowResBuffers.fbo);
		glBindTexture(GL_TEXTURE_2D, lowResBuffers.textureObject);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lowResBuffers.textureObject, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}
