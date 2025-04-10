#pragma once

#include "_componentBase.hpp"

#include <commonTypes/componentMappers.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <memory>
#include <functional>

namespace Components
{
	struct SoundBuffer;
	struct SoundDetails;

	struct Sound : ComponentBase
	{
		Sound(CM::SoundBuffer soundBuffer);
		~Sound();

		std::function<void()> soundTeardownF;

		Sound& play();
		Sound& stop();
		Sound& pause();

		Sound& setRemoveOnStop(bool value);

		Sound& setRelativeToAudioListener(bool value);
		Sound& setLooping(bool value);
		Sound& setVolume(float value);
		Sound& setPitch(float value);
		Sound& setPosition(glm::vec3 pos);
		Sound& setPosition(glm::vec2 pos);
		Sound& setMinDistance(float value);
		Sound& setAttenuation(float value);
		Sound& setPlayingOffset(float value);

		bool isRelativeToAudioListener() const;
		bool isStopped() const;
		bool isPaused() const;
		bool isPlaying() const;

		void step() override;

		void immediateFreeResources();

		static unsigned getNumOfInstances();

	private:
		static inline unsigned numOfInstances = 0;

		const float maxVolume;
		std::unique_ptr<SoundDetails> details;
		bool removeOnStop = false;
		bool loop = false;
	};
}
