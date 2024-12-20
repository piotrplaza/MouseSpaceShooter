#include "soundBuffer.hpp"

#include <SFML/Audio/SoundBuffer.hpp>

namespace Components
{
	struct SoundBufferDetails
	{
		sf::SoundBuffer sfSoundBuffer;
	};

	SoundBuffer::SoundBuffer(std::string path, float maxVolume) :
		details(std::make_unique<SoundBufferDetails>()),
		maxVolume(maxVolume)
	{
		if (!details->sfSoundBuffer.loadFromFile(path))
		{
			assert(!"unable to load sound");
			throw std::runtime_error("Unable to load sound \"" + path + "\".");
		}
		state = ComponentState::Ongoing;
	}

	SoundBuffer::~SoundBuffer() = default;

	sf::SoundBuffer& SoundBuffer::getBuffer()
	{
		return details->sfSoundBuffer;
	}

	const sf::SoundBuffer& SoundBuffer::getBuffer() const
	{
		return details->sfSoundBuffer;
	}

	float SoundBuffer::getMaxVolume() const
	{
		return maxVolume;
	}
}
