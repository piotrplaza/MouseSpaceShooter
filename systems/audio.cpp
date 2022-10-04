#include "audio.hpp"

#include <components/music.hpp>
#include <components/sound.hpp>
#include <components/soundBuffer.hpp>

#include <globals/components.hpp>

#include <SFML/Audio.hpp>

namespace Systems
{
	Audio::Audio() = default;

	void Audio::postInit() const
	{
	}

	void Audio::step() const
	{
		for (auto& music : Globals::Components().musics())
			if (music.step)
				music.step(music);

		for (auto& sound : Globals::Components().sounds())
			sound.step();
	}
}
