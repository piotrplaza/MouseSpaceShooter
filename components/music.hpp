#pragma once

#include "_componentBase.hpp"

#include <string>
#include <memory>
#include <functional>

namespace Components
{
	struct MusicDetails;

	struct Music : ComponentBase
	{
		Music(std::string path, float maxVolume = 1.0f);
		~Music();

		std::function<void(Music&)> step;

		void play();
		void stop();

		void loop(bool value);
		void volume(float value);

	private:
		std::unique_ptr<MusicDetails> details;
		const float maxVolume;
	};
}
