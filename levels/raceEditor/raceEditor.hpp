#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class RaceEditor: public Level
	{
	public:
		RaceEditor();
		~RaceEditor();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
