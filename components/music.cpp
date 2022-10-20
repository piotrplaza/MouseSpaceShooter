#include "music.hpp"

#include <SFML/Audio.hpp>

#include <algorithm>

namespace Components
{
	struct MusicDetails
	{
		sf::Music sfMusic;
	};

	Music::Music(std::string path, float maxVolume) :
		details(std::make_unique<MusicDetails>()),
		maxVolume(maxVolume)
	{
		details->sfMusic.openFromFile(path);
		volume(1.0f);
		loop(true);
		state = ComponentState::Ongoing;
	}

	Music::~Music() = default;

	void Music::play()
	{
		details->sfMusic.play();
	}

	void Music::stop()
	{
		details->sfMusic.stop();
	}

	void Music::loop(bool value)
	{
		details->sfMusic.setLoop(value);
	}

	void Music::volume(float value)
	{
		details->sfMusic.setVolume(std::clamp(value, 0.0f, 1.0f) * maxVolume * 100.0f);
	}
}
