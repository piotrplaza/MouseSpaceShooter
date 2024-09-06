#pragma once

#include "_componentBase.hpp"

#include <glm/glm.hpp>

#include <string>

namespace Components
{
	struct AudioListener : ComponentBase
	{
		enum class Positioning { Camera2D, Camera3D, Unbound };

		AudioListener();

		void setEnabled(bool value) override;

		void setVolume(float value);
		float getVolume() const;

		void setPosition(glm::vec3 value);
		glm::vec3 getPosition() const;

		void setDirection(glm::vec3 value);
		glm::vec3 getDirection() const;

		void setUpVector(glm::vec3 value);
		glm::vec3 getUpVector() const;

		void setPositioning(Positioning value);
		Positioning getPositioning() const;

	private:
		float savedVolume;
		Positioning positioning = Positioning::Camera2D;
	};
}
