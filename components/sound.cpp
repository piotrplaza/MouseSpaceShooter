#include "sound.hpp"

#include "soundBuffer.hpp"
#include "music.hpp"
#include "audioListener.hpp"

#include <globals/components.hpp>

#include <tools/utility.hpp>

#include <SFML/Audio/Sound.hpp>

#include <algorithm>

namespace Components
{
	struct SoundDetails
	{
		sf::Sound sfSound;
	};

	Sound::Sound(ComponentId soundBufferId):
		details(Sound::getNumOfInstances() + Music::getNumOfInstances() < 256 ? std::make_unique<SoundDetails>() : nullptr),
		maxVolume(Globals::Components().soundsBuffers()[soundBufferId].getMaxVolume())
	{
		if (!details)
		{
			Tools::PrintWarning("Audio limit was exceeded. Dummy sound was allocated.");
			return;
		}

		auto& soundBuffer = Globals::Components().soundsBuffers()[soundBufferId];
		details->sfSound.setBuffer(soundBuffer.getBuffer());

		setVolume(1.0f);
		setMinDistance(4.0f);
		setAttenuation(0.2f);

		state = ComponentState::Ongoing;

		++numOfInstances;
	}

	Sound::~Sound()
	{
		if (tearDownF)
			tearDownF();

		if (!details)
			return;

		--numOfInstances;
	}

	void Sound::play()
	{
		if (!details)
			return;

		details->sfSound.play();
	}

	void Sound::stop()
	{
		if (removeOnStop)
			state = ComponentState::Outdated;

		if (!details)
			return;

		details->sfSound.stop();
	}

	void Sound::setRemoveOnStop(bool value)
	{
		removeOnStop = value;
	}

	void Sound::setRelativeToAudioListener(bool value)
	{
		if (!details)
			return;

		details->sfSound.setRelativeToListener(value);
	}

	void Sound::setLoop(bool value)
	{
		loop = value;

		if (!details)
		{
			if (value)
				Tools::PrintError("Loop for dummy sound was set. It may cause permanent issue.");

			return;
		}

		details->sfSound.setLoop(value);
	}

	void Sound::setVolume(float value)
	{
		if (!details)
			return;

		details->sfSound.setVolume(std::clamp(value, 0.0f, 1.0f) * maxVolume * 100.0f);
	}

	void Sound::setPitch(float value)
	{
		if (!details)
			return;

		details->sfSound.setPitch(value);
	}

	void Sound::setPosition(glm::vec3 pos)
	{
		if (!details)
			return;

		details->sfSound.setPosition(pos.x, pos.y, pos.z);
	}

	void Sound::setPosition(glm::vec2 pos)
	{
		if (!details)
			return;

		details->sfSound.setPosition(pos.x, pos.y, 0.0f);
	}

	void Sound::setMinDistance(float value)
	{
		if (!details)
			return;

		details->sfSound.setMinDistance(value);
	}

	void Sound::setAttenuation(float value)
	{
		if (!details)
			return;

		details->sfSound.setAttenuation(value);
	}

	bool Sound::isRelativeToAudioListener() const
	{
		if (!details)
			return false;

		return details->sfSound.isRelativeToListener();
	}

	bool Sound::isStopped() const
	{
		if (!details)
			return true;

		return details->sfSound.getStatus() == sf::SoundSource::Stopped;
	}

	bool Sound::isPaused() const
	{
		if (!details)
			return false;

		return details->sfSound.getStatus() == sf::SoundSource::Paused;
	}

	bool Sound::isPlaying() const
	{
		if (!details)
			return false;

		return details->sfSound.getStatus() == sf::SoundSource::Playing;
	}

	void Sound::step()
	{
		if (removeOnStop && !loop && isStopped())
			state = ComponentState::Outdated;

		if (stepF)
			stepF(*this);
	}

	void Sound::immediateFreeResources()
	{
		if (tearDownF)
		{
			tearDownF();
			tearDownF = nullptr;
		}

		details.reset();
		state = ComponentState::Outdated;
		--numOfInstances;
	}

	unsigned Sound::getNumOfInstances()
	{
		return numOfInstances;
	}
}
