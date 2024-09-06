#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Particles: public Level
	{
	public:
		Particles();
		~Particles();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
