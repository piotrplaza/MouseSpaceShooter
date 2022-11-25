#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class SplineTest: public Level
	{
	public:
		SplineTest();
		~SplineTest();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
