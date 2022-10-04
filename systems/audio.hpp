#pragma once

namespace Systems
{
	class Audio
	{
	public:
		Audio();

		void postInit() const;
		void step() const;
	};
}
