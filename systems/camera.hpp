#pragma once

namespace Systems
{
	class Camera
	{
	public:
		Camera();

		void postInit() const;
		void step(bool paused = false) const;

	private:
		void step2D(bool paused) const;
		void step3D(bool paused) const;
	};
}
