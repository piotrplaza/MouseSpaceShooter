#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Collisions : public Level
	{
	public:
		Collisions();
		~Collisions();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
