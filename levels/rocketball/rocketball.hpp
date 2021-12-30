#pragma once

#include "../level.hpp"

#include <memory>

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
