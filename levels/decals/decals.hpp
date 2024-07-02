#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Decals: public Level
	{
	public:
		Decals();

		void postSetup() override;
		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
