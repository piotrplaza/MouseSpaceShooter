#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Paint: public Level
	{
	public:
		Paint();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
