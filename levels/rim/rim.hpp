#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Rim: public Level
	{
	public:
		Rim();
		~Rim();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
