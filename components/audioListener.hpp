#pragma once

#include "_componentBase.hpp"

#include <glm/glm.hpp>

#include <string>

namespace Components
{
	struct Camera2D;

	struct AudioListener : ComponentBase
	{
		enum class Positioning { Camera2D, Camera3D, Unbound };

		AudioListener();

		void setEnabled(bool value) override;
		void forceDisabled(bool value);

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

		std::function<glm::vec3(const Camera2D&)> camera2DTo3DPosF;

	private:
		float savedVolume;
		bool forceDisabled_ = false;
		Positioning positioning;
	};
}
