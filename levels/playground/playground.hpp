#pragma once

#include <memory>

#include "../level.hpp"

namespace Levels
{
	class Playground: public Level
	{
	public:
		Playground();
		~Playground();

	private:
		void setGraphicsSettings() const;
		void setTextures() const;
		void setPlayers() const;
		void setBackground() const;
		void setStaticWalls() const;
		void setDynamicWalls() const;
		void setGrapples() const;
		void setCamera() const;

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
