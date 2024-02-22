#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Crosses3DTexturing: public Level
	{
	public:
		Crosses3DTexturing();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
