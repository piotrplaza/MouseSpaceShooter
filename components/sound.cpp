#include "sound.hpp"

#include "soundBuffer.hpp"
#include "music.hpp"
#include "camera.hpp"

#include <globals/components.hpp>

#include <SFML/Audio.hpp>

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
			return;

		auto& soundBuffer = Globals::Components().soundsBuffers()[soundBufferId];

		details->sfSound.setBuffer(soundBuffer.getBuffer());

		volume(1.0f);
		minDistance(2.0f);
		attenuation(1.0f);
		state = ComponentState::Ongoing;

		++numOfInstances;
	}

	Sound::~Sound()
	{
		if (!details)
			return;

		if (tearDownF)
			tearDownF();

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
		if (!details)
			return;

		details->sfSound.stop();
	}

	void Sound::removeOnStop(bool value)
	{
		removeOnStop_ = value;
	}

	void Sound::loop(bool value)
	{
		if (!details)
			return;

		details->sfSound.setLoop(value);
	}

	void Sound::volume(float value)
	{
		if (!details)
			return;

		details->sfSound.setVolume(std::clamp(value, 0.0f, 1.0f) * maxVolume * 100.0f);
	}

	void Sound::pitch(float value)
	{
		if (!details)
			return;

		details->sfSound.setPitch(value);
	}

	void Sound::position(glm::vec3 pos)
	{
		if (!details)
			return;

		details->sfSound.setPosition(pos.x, pos.y, pos.z);
	}

	void Sound::position(glm::vec2 pos)
	{
		if (!details)
			return;

		details->sfSound.setPosition(pos.x, pos.y, Globals::Components().camera().details.projectionHSize * zFactor_);
	}

	void Sound::zFactor(float value)
	{
		zFactor_ = value;
	}

	void Sound::minDistance(float value)
	{
		if (!details)
			return;

		details->sfSound.setMinDistance(value);
	}

	void Sound::attenuation(float value)
	{
		if (!details)
			return;

		details->sfSound.setAttenuation(value);
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
		if (removeOnStop_ && isStopped())
			state = ComponentState::Outdated;

		if (stepF)
			stepF(*this);
	}

	void Sound::immediateFreeResources()
	{
		if (tearDownF)
			tearDownF();

		details.reset();
		state = ComponentState::Outdated;
		--numOfInstances;
	}

	unsigned Sound::getNumOfInstances()
	{
		return numOfInstances;
	}
}
