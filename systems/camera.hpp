#pragma once

namespace Systems
{
	class Camera
	{
	public:
		Camera();

		void postInit() const;
		void step() const;

	private:
		void postInit3D() const;
		void step3D() const;
	};
}
