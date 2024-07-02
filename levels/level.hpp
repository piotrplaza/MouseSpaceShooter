#pragma once

namespace Levels
{
	class Level
	{
	public:
		Level();
		virtual ~Level();

		virtual void postSetup() {}
		virtual void step() {}
	};
}
