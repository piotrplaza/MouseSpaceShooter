#pragma once

namespace Systems
{
	class Camera
	{
	public:
		Camera();

		void step();

	private:
		bool firstStep = true;
	};
}
