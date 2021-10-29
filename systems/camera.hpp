#pragma once

namespace Systems
{
	class Camera
	{
	public:
		Camera();

		void postInit() const;
		void step() const;
	};
}
