#pragma once

#include "../level.hpp"

namespace Levels
{
	class Tests: public Level
	{
	public:
		Tests();
		~Tests();

		void step() override;
	};
}
