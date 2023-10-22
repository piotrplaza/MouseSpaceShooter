#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Basic3D: public Level
	{
	public:
		Basic3D();
		~Basic3D();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
