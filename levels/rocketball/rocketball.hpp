#pragma once

#include <memory>

#include "../level.hpp"

namespace Levels
{
	class Rocketball: public Level
	{
	public:
		Rocketball();
		~Rocketball();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
