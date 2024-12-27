#include "level.hpp"

namespace Levels
{
	Level::Level()
	{
		ongoing = true;
	}

	Level::~Level()
	{
		ongoing = false;
	}
}
