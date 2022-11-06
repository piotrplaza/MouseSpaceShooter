#include "music.hpp"

#include "sound.hpp"

#include <tools/utility.hpp>

#include <SFML/Audio.hpp>

#include <algorithm>

namespace Components
{
	struct MusicDetails
	{
		sf::Music sfMusic;
	};

	Music::Music(std::string path, float maxVolume) :
		details(Sound::getNumOfInstances() + Music::getNumOfInstances() < 256 ? std::make_unique<MusicDetails>() : nullptr),
		maxVolume(maxVolume)
	{
		if (!details)
		{
			Tools::PrintWarning("Audio limit was exceeded. Dummy music was allocated.");
			return;
		}

		details->sfMusic.openFromFile(path);
		volume(1.0f);
		loop(true);
		state = ComponentState::Ongoing;

		++numOfInstances;
	}

	Music::~Music()
	{
		--numOfInstances;
	}

	void Music::play()
	{
		if (!details)
			return;

		details->sfMusic.play();
	}

	void Music::stop()
	{
		if (!details)
			return;

		details->sfMusic.stop();
	}

	void Music::loop(bool value)
	{
		if (!details)
		{
			if (value)
				Tools::PrintError("Loop for dummy music was set. It may cause permanent issue.");

			return;
		}

		details->sfMusic.setLoop(value);
	}

	void Music::volume(float value)
	{
		if (!details)
			return;

		details->sfMusic.setVolume(std::clamp(value, 0.0f, 1.0f) * maxVolume * 100.0f);
	}

	unsigned Music::getNumOfInstances()
	{
		return numOfInstances;
	}
}
