#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Particles3D: public Level
	{
	public:
		Particles3D();
		~Particles3D();

		void postSetup() override;
		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
