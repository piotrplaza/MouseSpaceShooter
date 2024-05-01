#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class SnakeCube: public Level
	{
	public:
		SnakeCube();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
