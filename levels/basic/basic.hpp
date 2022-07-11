#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Basic: public Level
	{
	public:
		Basic();
		~Basic();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
