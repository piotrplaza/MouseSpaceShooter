#include "sound.hpp"

#include "soundBuffer.hpp"
#include "music.hpp"
#include "audioListener.hpp"

#include <globals/components.hpp>

#include <tools/utility.hpp>

#include <SFML/Audio/Sound.hpp>

#include <algorithm>
#include <stdexcept>

namespace Components
{
	struct SoundDetails
	{
		sf::Sound sfSound;
	};

	Sound::Sound(CM::SoundBuffer soundBuffer):
		maxVolume(soundBuffer.component->getMaxVolume()),
		details(Sound::getNumOfInstances() + Music::getNumOfInstances() < 256 ? std::make_unique<SoundDetails>() : nullptr)
	{
		if (!details)
		{
			Tools::PrintWarning("Audio limit was exceeded. Dummy sound was allocated.");
			return;
		}

		details->sfSound.setBuffer(soundBuffer.component->getBuffer());

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

	Sound& Sound::play()
	{
		if (!details)
			return *this;

		details->sfSound.play();

		return *this;
	}

	Sound& Sound::stop()
	{
		if (removeOnStop)
			state = ComponentState::Outdated;

		if (!details)
			return *this;

		details->sfSound.stop();

		return *this;
	}

	Sound& Sound::pause()
	{
		if (!details)
			return *this;

		details->sfSound.pause();

		return *this;
	}

	Sound& Sound::setRemoveOnStop(bool value)
	{
		removeOnStop = value;

		return *this;
	}

	Sound& Sound::setRelativeToAudioListener(bool value)
	{
		if (!details)
			return *this;

		details->sfSound.setRelativeToListener(value);

		return *this;
	}

	Sound& Sound::setLoop(bool value)
	{
		loop = value;

		if (!details)
		{
			if (value)
				Tools::PrintError("Loop for dummy sound was set. It may cause permanent issue.");

			return *this;
		}

		details->sfSound.setLoop(value);

		return *this;
	}

	Sound& Sound::setVolume(float value)
	{
		if (!details)
			return *this;

		details->sfSound.setVolume(std::clamp(value, 0.0f, 1.0f) * maxVolume * 100.0f);

		return *this;
	}

	Sound& Sound::setPitch(float value)
	{
		if (!details)
			return *this;

		details->sfSound.setPitch(value);

		return *this;
	}

	Sound& Sound::setPosition(glm::vec3 pos)
	{
		if (!details)
			return *this;

		details->sfSound.setPosition(pos.x, pos.y, pos.z);

		return *this;
	}

	Sound& Sound::setPosition(glm::vec2 pos)
	{
		if (!details)
			return *this;

		details->sfSound.setPosition(pos.x, pos.y, 0.0f);

		return *this;
	}

	Sound& Sound::setMinDistance(float value)
	{
		if (!details)
			return *this;

		details->sfSound.setMinDistance(value);

		return *this;
	}

	Sound& Sound::setAttenuation(float value)
	{
		if (!details)
			return *this;

		details->sfSound.setAttenuation(value);

		return *this;
	}

	Sound& Sound::setPlayingOffset(float seconds)
	{
		if (!details)
			return *this;

		details->sfSound.setPlayingOffset(sf::seconds(seconds));

		return *this;
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
			stepF();
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
