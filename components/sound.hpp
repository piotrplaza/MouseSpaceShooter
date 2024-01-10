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

		std::function<void(Sound&)> stepF;
		std::function<void()> tearDownF;

		void play();
		void stop();

		void setRemoveOnStop(bool value);

		void setRelativeToAudioListener(bool value);
		void setLoop(bool value);
		void setVolume(float value);
		void setPitch(float value);
		void setPosition(glm::vec3 pos);
		void setPosition(glm::vec2 pos);
		void setMinDistance(float value);
		void setAttenuation(float value);

		bool isRelativeToAudioListener() const;
		bool isStopped() const;
		bool isPaused() const;
		bool isPlaying() const;

		void step();

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
