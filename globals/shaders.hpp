#pragma once

#include <memory>

namespace Shaders::Programs
{
	struct Basic;
	struct Colored;
	struct Julia;
	struct Particles;
	struct Textured;
	struct TexturedColorThreshold;
}

namespace Globals
{
	class Shaders
	{
	public:
		::Shaders::Programs::Basic& basic();
		::Shaders::Programs::Colored& colored();
		::Shaders::Programs::Julia& julia();
		::Shaders::Programs::Particles& particles();
		::Shaders::Programs::Textured& textured();
		::Shaders::Programs::TexturedColorThreshold& texturedColorThreshold();

	private:
		std::unique_ptr<::Shaders::Programs::Basic> basic_ = std::make_unique<::Shaders::Programs::Basic>();
		std::unique_ptr<::Shaders::Programs::Colored> colored_ = std::make_unique<::Shaders::Programs::Colored>();
		std::unique_ptr<::Shaders::Programs::Julia> julia_ = std::make_unique<::Shaders::Programs::Julia>();
		std::unique_ptr<::Shaders::Programs::Particles> particles_ = std::make_unique<::Shaders::Programs::Particles>();
		std::unique_ptr<::Shaders::Programs::Textured> textured_ = std::make_unique<::Shaders::Programs::Textured>();
		std::unique_ptr<::Shaders::Programs::TexturedColorThreshold> texturedColorThreshold_ = std::make_unique<::Shaders::Programs::TexturedColorThreshold>();
	};

	void InitializeShaders();

	class Shaders& Shaders();
}
