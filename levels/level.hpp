#pragma once

namespace Levels
{
	class Level
	{
	public:
		virtual ~Level();

		virtual void postSetup() {}
		virtual void step() {}
	};
}
