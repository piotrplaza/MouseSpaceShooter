#pragma once

#include "_componentBase.hpp"

#include <glm/glm.hpp>

#include <string>

namespace Components
{
	struct AudioListener : ComponentBase
	{
		AudioListener();

		void setEnable(bool value) override;

		void setVolume(float value);
		float getVolume() const;

		void setPosition(glm::vec3 value);
		glm::vec3 getPosition() const;

		void setDirection(glm::vec3 value);
		glm::vec3 getDirection() const;

		void setUpVector(glm::vec3 value);
		glm::vec3 getUpVector() const;

	private:
		float savedVolume;
	};
}
