#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Crosses3DTextured: public Level
	{
	public:
		Crosses3DTextured();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
