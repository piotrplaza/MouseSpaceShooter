#pragma once

namespace Levels
{
	class Level
	{
	public:
		Level();
		virtual ~Level();

		virtual void step() = 0;
	};
}
