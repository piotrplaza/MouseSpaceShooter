#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Particles2D: public Level
	{
	public:
		Particles2D();
		~Particles2D();

		void postSetup() override;
		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
