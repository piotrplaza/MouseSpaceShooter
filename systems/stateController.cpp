#include "stateController.hpp"

#include "tools/utility.hpp"

#include <globals.hpp>

#include <components/player.hpp>
#include <components/grapple.hpp>
#include <components/screenInfo.hpp>
#include <components/mouseState.hpp>
#include <components/graphicsSettings.hpp>
#include <components/framebuffers.hpp>
#include <components/texture.hpp>
#include <components/functor.hpp>

namespace
{
	constexpr int lowerResDivisor = 2;
	constexpr int lowestResDivisor = 4;
	constexpr int pixelArtShorterDim = 100;
	constexpr int lowPixelArtShorterDim = 30;
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

		createFramebuffers();
	}

	void StateController::frameSetup() const
	{
		using namespace Globals::Components;

		for (auto& [id, frameSetup] : frameSetups)
			frameSetup();
	}

	void StateController::frameTeardown() const
	{
		using namespace Globals::Components;

		for (auto& [id, frameTeardown] : frameTeardowns)
			frameTeardown();

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

		auto setTextureFramebufferSize = [&](Components::Framebuffers::SubBuffers& subBuffers, glm::ivec2 size)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, subBuffers.fbo);
			glBindTexture(GL_TEXTURE_2D, subBuffers.textureObject);
			subBuffers.size = size;
			textures[subBuffers.textureUnit - GL_TEXTURE0].size = subBuffers.size;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, subBuffers.size.x, subBuffers.size.y, 0, GL_RGBA, GL_FLOAT, nullptr);
			assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		};

		const glm::ivec2 pixelArtTextureFramebufferSize = size.x > size.y
			? glm::ivec2((int)(pixelArtShorterDim * (float)size.x / size.y), pixelArtShorterDim)
			: glm::ivec2(pixelArtShorterDim, (int)(pixelArtShorterDim * (float)size.y / size.x));

		const glm::ivec2 lowPixelArtTextureFramebufferSize = size.x > size.y
			? glm::ivec2((int)(lowPixelArtShorterDim * (float)size.x / size.y), lowPixelArtShorterDim)
			: glm::ivec2(lowPixelArtShorterDim, (int)(lowPixelArtShorterDim * (float)size.y / size.x));

		setTextureFramebufferSize(framebuffers.main, size);
		setTextureFramebufferSize(framebuffers.lowerLinearBlend0, size / lowerResDivisor);
		setTextureFramebufferSize(framebuffers.lowerLinearBlend1, size / lowerResDivisor);
		setTextureFramebufferSize(framebuffers.lowestLinearBlend0, size / lowestResDivisor);
		setTextureFramebufferSize(framebuffers.lowestLinearBlend1, size / lowestResDivisor);
		setTextureFramebufferSize(framebuffers.pixelArtBlend0, pixelArtTextureFramebufferSize);
		setTextureFramebufferSize(framebuffers.pixelArtBlend1, pixelArtTextureFramebufferSize);
		setTextureFramebufferSize(framebuffers.lowPixelArtBlend0, lowPixelArtTextureFramebufferSize);
		setTextureFramebufferSize(framebuffers.lowPixelArtBlend1, lowPixelArtTextureFramebufferSize);
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

	void StateController::createFramebuffers() const
	{
		using namespace Globals::Components;

		auto createTextureFramebuffer = [](Components::Framebuffers::SubBuffers& subBuffers)
		{
			glGenFramebuffers(1, &subBuffers.fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, subBuffers.fbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, subBuffers.textureObject, 0);
		};

		createTextureFramebuffer(framebuffers.main);
		createTextureFramebuffer(framebuffers.lowerLinearBlend0);
		createTextureFramebuffer(framebuffers.lowerLinearBlend1);
		createTextureFramebuffer(framebuffers.lowestLinearBlend0);
		createTextureFramebuffer(framebuffers.lowestLinearBlend1);
		createTextureFramebuffer(framebuffers.pixelArtBlend0);
		createTextureFramebuffer(framebuffers.pixelArtBlend1);
		createTextureFramebuffer(framebuffers.lowPixelArtBlend0);
		createTextureFramebuffer(framebuffers.lowPixelArtBlend1);
	}
}
