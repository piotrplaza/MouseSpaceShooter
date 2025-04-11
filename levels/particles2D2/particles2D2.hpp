#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Particles2D2: public Level
	{
	public:
		Particles2D2();
		~Particles2D2();

		void postSetup() override;
		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
