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

		void postSetup() override;
		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
