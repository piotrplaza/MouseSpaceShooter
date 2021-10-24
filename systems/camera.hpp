#pragma once

namespace Systems
{
	class Camera
	{
	public:
		Camera();

		void initializationFinalize() const;
		void step() const;
	};
}
