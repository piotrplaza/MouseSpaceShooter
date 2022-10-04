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

		void play();

		void removeOnStop(bool value);

		void loop(bool value);
		void volume(float value);
		void pitch(float value);
		void position(glm::vec3 pos);
		void position(glm::vec2 pos);
		void zFactor(float value);
		void minDistance(float value);
		void setAttenuation(float value);

		bool isStopped() const;
		bool isPaused() const;
		bool isPlaying() const;

		void step();

	private:
		std::unique_ptr<SoundDetails> details;
		bool removeOnStop_ = false;
		float zFactor_ = 20.0f;
	};
}
