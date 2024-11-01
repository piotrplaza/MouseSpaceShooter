#pragma once

#include "_componentBase.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <string>
#include <memory>
#include <functional>

namespace Components
{
	struct MusicDetails;

	struct Music : ComponentBase
	{
		static unsigned getNumOfInstances();

		Music(std::string path, float maxVolume = 1.0f);
		~Music();

		std::function<void(Music&)> step;

		Music& play();
		Music& stop();
		Music& pause();

		Music& setRelativeToAudioListener(bool value);
		Music& setLoop(bool value);
		Music& setVolume(float value);
		Music& setPitch(float value);
		Music& setPosition(glm::vec3 pos);
		Music& setPosition(glm::vec2 pos);
		Music& setMinDistance(float value);
		Music& setAttenuation(float value);
		Music& setPlayingOffset(float seconds);

		bool isRelativeToAudioListener() const;
		bool isStopped() const;
		bool isPaused() const;
		bool isPlaying() const;

		const std::string& getPath() const;

	private:
		static inline unsigned numOfInstances = 0;

		std::unique_ptr<MusicDetails> details;
		const std::string path;
		const float maxVolume;
	};
}
