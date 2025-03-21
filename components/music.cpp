#include "music.hpp"

#include "sound.hpp"

#include <tools/utility.hpp>

#include <SFML/Audio/Music.hpp>

#include <algorithm>

namespace Components
{
	struct MusicDetails
	{
		sf::Music sfMusic;
	};

	unsigned Music::getNumOfInstances()
	{
		return numOfInstances;
	}

	Music::Music(std::string path, float maxVolume) :
		details(Sound::getNumOfInstances() + Music::getNumOfInstances() < 256 ? std::make_unique<MusicDetails>() : nullptr),
		path(std::move(path)),
		maxVolume(maxVolume)
	{
		if (!details)
		{
			Tools::PrintWarning("Audio limit was exceeded. Dummy music was allocated.");
			return;
		}

		if (!details->sfMusic.openFromFile(this->path))
		{
			assert(!"unable to load music");
			throw std::runtime_error("Unable to load music \"" + this->path + "\".");
		}

		details->sfMusic.setSpatializationEnabled(false);

		setVolume(1.0f);
		setLoop(true);
		state = ComponentState::Ongoing;

		++numOfInstances;
	}

	Music::~Music()
	{
		--numOfInstances;
	}

	Music& Music::play()
	{
		if (!details)
			return *this;

		details->sfMusic.play();

		return *this;
	}

	Music& Music::stop()
	{
		if (!details)
			return *this;

		details->sfMusic.stop();

		return *this;
	}

	Music& Music::pause()
	{
		if (!details)
			return *this;

		details->sfMusic.pause();

		return *this;
	}

	Music& Music::setRelativeToAudioListener(bool value)
	{
		if (!details)
			return *this;

		details->sfMusic.setRelativeToListener(value);

		return *this;
	}

	Music& Music::setLoop(bool value)
	{
		if (!details)
		{
			if (value)
				Tools::PrintError("Loop for dummy music was set. It may cause permanent issue.");

			return *this;
		}

		details->sfMusic.setLooping(value);

		return *this;
	}

	Music& Music::setVolume(float value)
	{
		if (!details)
			return *this;

		details->sfMusic.setVolume(std::clamp(value, 0.0f, 1.0f) * maxVolume * 100.0f);

		return *this;
	}

	Music& Music::setPitch(float value)
	{
		if (!details)
			return *this;

		details->sfMusic.setPitch(value);

		return *this;
	}

	Music& Music::setPosition(glm::vec3 pos)
	{
		if (!details)
			return *this;

		details->sfMusic.setPosition({ pos.x, pos.y, pos.z });

		return *this;
	}

	Music& Music::setPosition(glm::vec2 pos)
	{
		if (!details)
			return *this;

		details->sfMusic.setPosition({ pos.x, pos.y, 0.0f });

		return *this;
	}

	Music& Music::setMinDistance(float value)
	{
		if (!details)
			return *this;

		details->sfMusic.setMinDistance(value);

		return *this;
	}

	Music& Music::setAttenuation(float value)
	{
		if (!details)
			return *this;

		details->sfMusic.setAttenuation(value);

		return *this;
	}

	Music& Music::setPlayingOffset(float seconds)
	{
		if (!details)
			return *this;

		details->sfMusic.setPlayingOffset(sf::seconds(seconds));

		return *this;
	}

	bool Music::isRelativeToAudioListener() const
	{
		if (!details)
			return false;

		return details->sfMusic.isRelativeToListener();
	}

	bool Music::isStopped() const
	{
		if (!details)
			return true;

		return details->sfMusic.getStatus() == sf::Music::Status::Stopped;
	}

	bool Music::isPaused() const
	{
		if (!details)
			return true;

		return details->sfMusic.getStatus() == sf::Music::Status::Paused;
	}

	bool Music::isPlaying() const
	{
		if (!details)
			return false;

		return details->sfMusic.getStatus() == sf::Music::Status::Playing;
	}

	const std::string& Music::getPath() const
	{
		return path;
	}
}
