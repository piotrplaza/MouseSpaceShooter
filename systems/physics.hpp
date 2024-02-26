#pragma once

#include <memory>

class b2World;

namespace Systems
{
	class Physics
	{
	public:
		Physics();
		~Physics();

		void postInit();
		void step();
		void pause();

	private:
		std::unique_ptr<b2World> world;
	};
}
