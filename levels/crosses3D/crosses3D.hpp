#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Crosses3D: public Level
	{
	public:
		Crosses3D();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
