#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Tmp: public Level
	{
	public:
		Tmp();
		~Tmp();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
