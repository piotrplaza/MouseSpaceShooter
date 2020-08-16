#pragma once

#include <memory>

#include "../level.hpp"

namespace Levels
{
	class Race1 : public Level
	{
	public:
		Race1();
		~Race1();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
