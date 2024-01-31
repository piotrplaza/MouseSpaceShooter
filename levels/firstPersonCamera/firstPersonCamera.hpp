#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class FirstPersonCamera: public Level
	{
	public:
		FirstPersonCamera();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
