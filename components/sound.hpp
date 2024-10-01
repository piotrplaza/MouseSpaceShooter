#pragma once

#include "_componentBase.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <memory>
#include <functional>

namespace Components
{
	struct SoundDetails;

	struct Sound : ComponentBase
	{
		Sound(ComponentId soundBufferId);
		~Sound();

		ComponentId soundBufferId = 0;

		std::function<void()> tearDownF;

		Sound& play();
		Sound& stop();
		Sound& pause();

		Sound& setRemoveOnStop(bool value);

		Sound& setRelativeToAudioListener(bool value);
		Sound& setLoop(bool value);
		Sound& setVolume(float value);
		Sound& setPitch(float value);
		Sound& setPosition(glm::vec3 pos);
		Sound& setPosition(glm::vec2 pos);
		Sound& setMinDistance(float value);
		Sound& setAttenuation(float value);
		Sound& setPlayingOffset(float seconds);

		bool isRelativeToAudioListener() const;
		bool isStopped() const;
		bool isPaused() const;
		bool isPlaying() const;

		void step() override;

		void immediateFreeResources();

		static unsigned getNumOfInstances();

	private:
		static inline unsigned numOfInstances = 0;
		std::unique_ptr<SoundDetails> details;
		const float maxVolume;
		bool removeOnStop = false;
		bool loop = false;
	};
}
