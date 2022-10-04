#include "soundBuffer.hpp"

#include <SFML/Audio.hpp>

namespace Components
{
	struct SoundBufferDetails
	{
		sf::SoundBuffer sfSoundBuffer;
	};

	SoundBuffer::SoundBuffer(std::string path) :
		details(std::make_unique<SoundBufferDetails>())
	{
		details->sfSoundBuffer.loadFromFile(path);
		state = ComponentState::Ongoing;
	}

	SoundBuffer::~SoundBuffer() = default;

	sf::SoundBuffer& SoundBuffer::getBuffer()
	{
		return details->sfSoundBuffer;
	}
}
