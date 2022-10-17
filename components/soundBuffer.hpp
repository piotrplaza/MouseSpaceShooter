#pragma once

#include "_componentBase.hpp"

#include <string>
#include <memory>

namespace sf
{
	class SoundBuffer;
}

namespace Components
{
	struct SoundBufferDetails;
	struct Sound;

	struct SoundBuffer : ComponentBase
	{
		friend Components::Sound;

		SoundBuffer(std::string path, float maxVolume = 1.0f);
		~SoundBuffer();

		float getMaxVolume() const;

	private:
		sf::SoundBuffer& getBuffer();
		std::unique_ptr<SoundBufferDetails> details;
		const float maxVolume;
	};
}
