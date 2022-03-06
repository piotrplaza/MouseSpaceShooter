#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Gravity: public Level
	{
	public:
		Gravity();
		~Gravity();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
