#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Crosses3DInstancing: public Level
	{
	public:
		Crosses3DInstancing();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
