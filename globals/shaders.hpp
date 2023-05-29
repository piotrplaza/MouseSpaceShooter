#pragma once

#include <memory>

namespace Shaders::Programs
{
	struct Basic;
	struct Julia;
	struct Particles;
	struct Textured;
	struct TexturedColorThreshold;
	struct TexturedPhong;
}

namespace Globals
{
	class ShadersHolder
	{
	public:
		Shaders::Programs::Basic& basic();
		Shaders::Programs::Julia& julia();
		Shaders::Programs::Particles& particles();
		Shaders::Programs::Textured& textured();
		Shaders::Programs::TexturedColorThreshold& texturedColorThreshold();
		Shaders::Programs::TexturedPhong& texturedPhong();

	private:
		std::unique_ptr<Shaders::Programs::Basic> basic_ = std::make_unique<Shaders::Programs::Basic>();
		std::unique_ptr<Shaders::Programs::Julia> julia_ = std::make_unique<Shaders::Programs::Julia>();
		std::unique_ptr<Shaders::Programs::Particles> particles_ = std::make_unique<Shaders::Programs::Particles>();
		std::unique_ptr<Shaders::Programs::Textured> textured_ = std::make_unique<Shaders::Programs::Textured>();
		std::unique_ptr<Shaders::Programs::TexturedColorThreshold> texturedColorThreshold_ = std::make_unique<Shaders::Programs::TexturedColorThreshold>();
		std::unique_ptr<Shaders::Programs::TexturedPhong> texturedPhong_ = std::make_unique<Shaders::Programs::TexturedPhong>();
	};

	void InitializeShaders();

	class ShadersHolder& Shaders();
}
