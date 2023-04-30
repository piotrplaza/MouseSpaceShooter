#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Race: public Level
	{
	public:
		Race();
		~Race();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
