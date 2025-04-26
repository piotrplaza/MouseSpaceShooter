#include "audio.hpp"

#include <components/music.hpp>
#include <components/sound.hpp>
#include <components/soundBuffer.hpp>
#include <components/audioListener.hpp>
#include <components/camera2D.hpp>
#include <components/camera3D.hpp>
#include <components/functor.hpp>

#include <globals/components.hpp>

#include <SFML/Audio.hpp>

namespace Systems
{
	Audio::Audio()
	{
		auto& audioListener = Globals::Components().audioListener();
		const auto& camera2D = Globals::Components().camera2D();
		audioListener.camera2DTo3DPosF = [camera2D](const Components::Camera2D& camera2D) { return glm::vec3(camera2D.details.position, camera2D.details.projectionHSize * 1.0f); };
	}

	void Audio::postInit() const
	{
	}

	void Audio::step() const
	{
		auto& audioListener = Globals::Components().audioListener();
		const auto& camera2D = Globals::Components().camera2D();
		switch(audioListener.getPositioning())
		{
			case Components::AudioListener::Positioning::Camera2D:
				audioListener.setPosition(audioListener.camera2DTo3DPosF(camera2D));
				audioListener.setDirection(glm::vec3(0.0f, 0.0f, -1.0f));
				audioListener.setUpVector(glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case Components::AudioListener::Positioning::Camera3D:
				// TODO
				break;
		}

		for (auto& music : Globals::Components().staticMusics())
			if (music.step)
				music.step(music);

		for (auto& music : Globals::Components().musics())
			if (music.step)
				music.step(music);

		for (auto& sound : Globals::Components().staticSounds())
			sound.step();

		for (auto& sound : Globals::Components().sounds())
			sound.step();
	}
}
