#include "sound.hpp"

#include "soundBuffer.hpp"
#include "camera.hpp"

#include <globals/components.hpp>

#include <SFML/Audio.hpp>

namespace Components
{
	struct SoundDetails
	{
		sf::Sound sfSound;
	};

	Sound::Sound(ComponentId soundBufferId):
		details(std::make_unique<SoundDetails>())
	{
		auto& soundsBuffers = Globals::Components().soundsBuffers();

		details->sfSound.setBuffer(soundsBuffers[soundBufferId].getBuffer());
		minDistance(2.0f);
		setAttenuation(1.0f);
		state = ComponentState::Ongoing;
	}

	Sound::~Sound() = default;

	void Sound::play()
	{
		details->sfSound.play();
	}

	void Sound::removeOnStop(bool value)
	{
		removeOnStop_ = value;
	}

	void Sound::loop(bool value)
	{
		details->sfSound.setLoop(value);
	}

	void Sound::volume(float value)
	{
		details->sfSound.setVolume(value * 100.0f);
	}

	void Sound::pitch(float value)
	{
		details->sfSound.setPitch(value);
	}

	void Sound::position(glm::vec3 pos)
	{
		details->sfSound.setPosition(pos.x, pos.y, pos.z);
	}

	void Sound::position(glm::vec2 pos)
	{
		details->sfSound.setPosition(pos.x, pos.y, Globals::Components().camera().details.projectionHSize / zFactor_);
	}

	void Sound::zFactor(float value)
	{
		zFactor_ = value;
	}

	void Sound::minDistance(float value)
	{
		details->sfSound.setMinDistance(value);
	}

	void Sound::setAttenuation(float value)
	{
		details->sfSound.setAttenuation(value);
	}

	bool Sound::isStopped() const
	{
		return details->sfSound.getStatus() == sf::SoundSource::Stopped;
	}

	bool Sound::isPaused() const
	{
		return details->sfSound.getStatus() == sf::SoundSource::Paused;
	}

	bool Sound::isPlaying() const
	{
		return details->sfSound.getStatus() == sf::SoundSource::Playing;
	}

	void Sound::step()
	{
		if (removeOnStop_ && isStopped())
			state = ComponentState::Outdated;

		if (stepF)
			stepF(*this);
	}
}
