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

		void removeOnStop(bool value);

		void loop(bool value);
		void volume(float value);
		void pitch(float value);
		void position(glm::vec3 pos);
		void position(glm::vec2 pos);
		void zFactor(float value);
		void minDistance(float value);
		void attenuation(float value);

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
		bool removeOnStop_ = false;
		bool loop_ = false;
		float zFactor_ = 0.05f;
	};
}
