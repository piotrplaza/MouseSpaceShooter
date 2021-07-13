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
	constexpr int pixelArtShorterDim = 100;
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

		createLowResFramebuffers();
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

		glBindFramebuffer(GL_FRAMEBUFFER, lowResBuffers.lowLinear.fbo);
		glBindTexture(GL_TEXTURE_2D, lowResBuffers.lowLinear.textureObject);
		lowResBuffers.lowLinear.size = size / lowResDivisor;
		textures[lowResBuffers.lowLinear.textureUnit - GL_TEXTURE0].size = lowResBuffers.lowLinear.size;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, lowResBuffers.lowLinear.size.x, lowResBuffers.lowLinear.size.y, 0, GL_RGBA, GL_FLOAT, nullptr);
		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		glBindFramebuffer(GL_FRAMEBUFFER, lowResBuffers.pixelArt.fbo);
		glBindTexture(GL_TEXTURE_2D, lowResBuffers.pixelArt.textureObject);
		lowResBuffers.pixelArt.size = size.x > size.y
			? glm::ivec2((int)(pixelArtShorterDim * (float)size.x / size.y), pixelArtShorterDim)
			: glm::ivec2(pixelArtShorterDim, (int)(pixelArtShorterDim * (float)size.y / size.x));
		textures[lowResBuffers.pixelArt.textureUnit - GL_TEXTURE0].size = lowResBuffers.pixelArt.size;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, lowResBuffers.pixelArt.size.x, lowResBuffers.pixelArt.size.y, 0, GL_RGBA, GL_FLOAT, nullptr);
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

	void StateController::createLowResFramebuffers() const
	{
		using namespace Globals::Components;

		glGenFramebuffers(1, &lowResBuffers.lowLinear.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, lowResBuffers.lowLinear.fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lowResBuffers.lowLinear.textureObject, 0);

		glGenFramebuffers(1, &lowResBuffers.pixelArt.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, lowResBuffers.pixelArt.fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lowResBuffers.pixelArt.textureObject, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}
