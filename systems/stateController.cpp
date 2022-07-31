#include "stateController.hpp"

#include "tools/utility.hpp"

#include <components/plane.hpp>
#include <components/grapple.hpp>
#include <components/screenInfo.hpp>
#include <components/keyboardState.hpp>
#include <components/mouseState.hpp>
#include <components/graphicsSettings.hpp>
#include <components/framebuffers.hpp>
#include <components/texture.hpp>
#include <components/functor.hpp>
#include <components/physics.hpp>

#include <ogl/shaders/textured.hpp>

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
		for(auto& plane: Globals::Components().planes())
			plane.details.previousCenter = plane.getCenter();

		for (auto& [id, grapple] : Globals::Components().grapples())
			grapple.details.previousCenter = grapple.getCenter();
	}

	void StateController::stepSetup() const
	{
		for (auto& [id, stepSetup] : Globals::Components().frameSetups())
			stepSetup();
	}

	void StateController::renderSetup() const
	{
		Globals::Shaders().textured().numOfPlayers(Globals::Components().planes().size());

		for (int i = 0; i < (int)Globals::Components().planes().size(); ++i)
			Globals::Shaders().textured().playersCenter(i, Globals::Components().planes()[i].getCenter());
	}

	void StateController::stepTeardown() const
	{
		for(auto& player: Globals::Components().planes())
			player.details.previousCenter = player.getCenter();

		for (auto& [id, grapple] : Globals::Components().grapples())
			grapple.details.previousCenter = grapple.getCenter();

		for (auto& [id, stepTeardown] : Globals::Components().frameTeardowns())
			stepTeardown();
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

			glBindRenderbuffer(GL_RENDERBUFFER, subBuffers.depthBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);

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
	}

	void StateController::handleMouseButtons()
	{
		auto& mouseState = Globals::Components().mouseState();

		auto updateButton = [&](bool Components::MouseState::Buttons::* button)
		{
			mouseState.pressed.*button = mouseState.pressing.*button && !(prevMouseKeys.*button);
			mouseState.released.*button = !(mouseState.pressing.*button) && prevMouseKeys.*button;
		};

		updateButton(&Components::MouseState::Buttons::lmb);
		updateButton(&Components::MouseState::Buttons::rmb);
		updateButton(&Components::MouseState::Buttons::mmb);
		updateButton(&Components::MouseState::Buttons::xmb1);
		updateButton(&Components::MouseState::Buttons::xmb2);

		mouseState.pressed.wheel = mouseState.pressing.wheel - prevMouseKeys.wheel;
		mouseState.released.wheel = mouseState.pressed.wheel;

		prevMouseKeys = mouseState.pressing;
	}

	void StateController::handleKeyboard(const std::array<bool, 256>& keys)
	{
		if (keys['P'] && !prevKeyboardKeys['P'])
			Globals::Components().physics().paused = !Globals::Components().physics().paused;

		auto& keyboardState = Globals::Components().keyboardState();
		for (size_t i = 0; i < keys.size(); ++i)
		{
			keyboardState.pressing[i] = keys[i];
			keyboardState.pressed[i] = keys[i] && !prevKeyboardKeys[i];
			keyboardState.released[i] = !keys[i] && prevKeyboardKeys[i];
		}

		prevKeyboardKeys = keys;
	}
}
