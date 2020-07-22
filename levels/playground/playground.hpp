#pragma once

#include <memory>

#include "../level.hpp"

namespace Levels
{
	class Playground: public Level
	{
	public:
		Playground();
		~Playground();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
