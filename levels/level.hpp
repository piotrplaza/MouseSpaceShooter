#pragma once

namespace Levels
{
	class Level
	{
	public:
		Level();
		~Level();

		virtual void step() = 0;
	};
}
