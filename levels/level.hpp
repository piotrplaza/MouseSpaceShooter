#pragma once

namespace Levels
{
	class Level
	{
	public:
		static inline bool ongoing = false;

		Level();
		virtual ~Level();

		virtual void postSetup() {}
		virtual void step() {}
	};
}
