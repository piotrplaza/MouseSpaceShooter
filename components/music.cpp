#include "music.hpp"

#include <SFML/Audio.hpp>

namespace Components
{
	struct MusicDetails
	{
		sf::Music sfMusic;
	};

	Music::Music(std::string path) :
		details(std::make_unique<MusicDetails>())
	{
		details->sfMusic.openFromFile(path);
		loop(true);
		state = ComponentState::Ongoing;
	}

	Music::~Music() = default;

	void Music::play()
	{
		details->sfMusic.play();
	}

	void Music::loop(bool value)
	{
		details->sfMusic.setLoop(value);
	}

	void Music::volume(float value)
	{
		details->sfMusic.setVolume(value * 100.0f);
	}
}
