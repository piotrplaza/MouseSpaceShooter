#include "audio.hpp"

#include <components/music.hpp>
#include <components/sound.hpp>
#include <components/soundBuffer.hpp>
#include <components/audioListener.hpp>
#include <components/camera2D.hpp>
#include <components/camera3D.hpp>

#include <globals/components.hpp>

#include <SFML/Audio.hpp>

namespace Systems
{
	Audio::Audio() = default;

	void Audio::postInit() const
	{
		Globals::Components().audioListener().setEnable(true);
	}

	void Audio::step() const
	{
		auto& audioListener = Globals::Components().audioListener();
		switch(audioListener.getPositioning())
		{
			case Components::AudioListener::Positioning::Camera2D:
				audioListener.setPosition(glm::vec3(Globals::Components().camera2D().details.position, Globals::Components().camera2D().getZ()));
				audioListener.setDirection(glm::vec3(0.0f, 0.0f, -1.0f));
				audioListener.setUpVector(glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case Components::AudioListener::Positioning::Camera3D:
				// TODO
				break;
		}

		for (auto& music : Globals::Components().musics())
			if (music.step)
				music.step(music);

		for (auto& sound : Globals::Components().sounds())
			sound.step();
	}
}
