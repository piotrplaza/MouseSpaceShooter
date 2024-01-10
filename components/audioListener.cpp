#include "audioListener.hpp"

#include <SFML/Audio/Listener.hpp>

namespace Components
{
	AudioListener::AudioListener()
	{
		savedVolume = getVolume();
		state = ComponentState::Ongoing;
	}

	void AudioListener::setEnable(bool value)
	{
		if (value == isEnabled())
			return;

		ComponentBase::setEnable(value);

		if (value)
			setVolume(savedVolume);
		else
		{
			savedVolume = getVolume();
			setVolume(0.0f);
		}
	}

	void AudioListener::setVolume(float value)
	{
		sf::Listener::setGlobalVolume(value * 100.0f);
	}

	float AudioListener::getVolume() const
	{
		return isEnabled() 
			? sf::Listener::getGlobalVolume() / 100.0f
			: savedVolume;
	}

	void AudioListener::setPosition(glm::vec3 value)
	{
		sf::Listener::setPosition(value.x, value.y, value.z);
	}

	glm::vec3 AudioListener::getPosition() const
	{
		sf::Vector3f pos = sf::Listener::getPosition();
		return { pos.x, pos.y, pos.z };
	}

	void AudioListener::setDirection(glm::vec3 value)
	{
		sf::Listener::setDirection(value.x, value.y, value.z);
	}

	glm::vec3 AudioListener::getDirection() const
	{
		sf::Vector3f dir = sf::Listener::getDirection();
		return { dir.x, dir.y, dir.z };
	}

	void AudioListener::setUpVector(glm::vec3 value)
	{
		sf::Listener::setUpVector(value.x, value.y, value.z);
	}

	glm::vec3 AudioListener::getUpVector() const
	{
		sf::Vector3f up = sf::Listener::getUpVector();
		return { up.x, up.y, up.z };
	}

	void AudioListener::setPositioning(Positioning value)
	{
		positioning = value;

		setPosition(glm::vec3(0.0f));
		setDirection({ 0.0f, 0.0f, -1.0f });
		setUpVector({ 0.0f, 1.0f, 0.0f });
	}

	AudioListener::Positioning AudioListener::getPositioning() const
	{
		return positioning;
	}
}
