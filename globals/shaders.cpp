#include "shaders.hpp"

#include "ogl/shaders/basic.hpp"
#include "ogl/shaders/julia.hpp"
#include "ogl/shaders/particles.hpp"
#include "ogl/shaders/textured.hpp"
#include "ogl/shaders/texturedColorThreshold.hpp"
#include "ogl/shaders/texturedPhong.hpp"

namespace Globals
{
	static std::unique_ptr<ShadersHolder> shadersHolder;

	Shaders::Programs::Basic& ShadersHolder::basic()
	{
		return *basic_;
	}

	Shaders::Programs::Julia& ShadersHolder::julia()
	{
		return *julia_;
	}

	Shaders::Programs::Particles& ShadersHolder::particles()
	{
		return *particles_;
	}

	Shaders::Programs::Textured& ShadersHolder::textured()
	{
		return *textured_;
	}

	Shaders::Programs::TexturedColorThreshold& ShadersHolder::texturedColorThreshold()
	{
		return *texturedColorThreshold_;
	}

	Shaders::Programs::TexturedPhong& ShadersHolder::texturedPhong()
	{
		return *texturedPhong_;
	}

	void InitializeShaders()
	{
		shadersHolder = std::make_unique<ShadersHolder>();
	}

	ShadersHolder& Shaders()
	{
		return *shadersHolder;
	}
}
