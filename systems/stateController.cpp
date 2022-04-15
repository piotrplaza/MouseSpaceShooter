#include "stateController.hpp"

#include "tools/utility.hpp"

#include <components/player.hpp>
#include <components/grapple.hpp>
#include <components/screenInfo.hpp>
#include <components/mouseState.hpp>
#include <components/graphicsSettings.hpp>
#include <components/framebuffers.hpp>
#include <components/texture.hpp>
#include <components/functor.hpp>

#include <ogl/shaders/textured.hpp>
#include <ogl/shaders/sceneCoordTextured.hpp>

#include <globals/components.hpp>
#include <globals/shaders.hpp>

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

	void StateController::postInit() const
	{
		Globals::ForEach(Globals::Components().players(), [](auto& player) {
			player.previousCenter = player.getCenter();
			});

		Globals::ForEach(Globals::Components().grapples(), [](auto& grapple) {
			grapple.previousCenter = grapple.getCenter();
			});
	}

	void StateController::frameSetup() const
	{
		for (auto& [id, frameSetup] : Globals::Components().frameSetups())
			frameSetup();
	}

	void StateController::renderSetup() const
	{
		Globals::Shaders().textured().numOfPlayers(Globals::Components().players().size() - 1);
		Globals::Shaders().sceneCoordTextured().numOfPlayers(Globals::Components().players().size() - 1);

		for (int i = 1; i < (int)Globals::Components().players().size(); ++i)
		{
			Globals::Shaders().textured().playersCenter(i - 1, Globals::Components().players()[i].getCenter());
			Globals::Shaders().sceneCoordTextured().playersCenter(i - 1, Globals::Components().players()[i].getCenter());
		}
	}

	void StateController::frameTeardown() const
	{
		Globals::ForEach(Globals::Components().players(), [](auto& player) {
			player.previousCenter = player.getCenter();
			});

		Globals::ForEach(Globals::Components().grapples(), [](auto& grapple) {
			grapple.previousCenter = grapple.getCenter();
			});

		for (auto& [id, frameTeardown] : Globals::Components().frameTeardowns())
			frameTeardown();
	}

	void StateController::changeWindowSize(glm::ivec2 size) const
	{
		auto& screenInfo = Globals::Components().screenInfo();
		auto& framebuffers = Globals::Components().framebuffers();

		screenInfo.windowSize = size;
		screenInfo.windowCenterInScreenSpace = { screenInfo.windowLocation + screenInfo.windowSize / 2 };

		auto setTextureFramebufferSize = [&](Components::Framebuffers::SubBuffers& subBuffers, glm::ivec2 size)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, subBuffers.fbo);
			glBindTexture(GL_TEXTURE_2D, subBuffers.textureObject);
			subBuffers.size = size;
			Globals::Components().textures()[subBuffers.textureUnit - GL_TEXTURE0].loaded.size = subBuffers.size;
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
		auto& screenInfo = Globals::Components().screenInfo();

		screenInfo.windowLocation = location;
		screenInfo.windowCenterInScreenSpace = { location + screenInfo.windowSize / 2 };
	}

	void StateController::resetMousePosition() const
	{
		Tools::SetMousePos(Globals::Components().screenInfo().windowCenterInScreenSpace);
		Globals::Components().mouseState().position = Globals::Components().screenInfo().windowCenterInScreenSpace;
	}

	void StateController::handleMousePosition() const
	{
		auto& mouseState = Globals::Components().mouseState();

		const auto prevPosition = mouseState.position;
		mouseState.position = Tools::GetMousePos();
		mouseState.delta = mouseState.position - prevPosition;

		resetMousePosition();
	}

	void StateController::handleKeyboard(bool const* const keys) const
	{
	}
}
