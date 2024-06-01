#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Noise: public Level
	{
	public:
		Noise();

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
