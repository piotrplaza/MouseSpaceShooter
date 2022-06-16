#include "shaders.hpp"

#include "ogl/shaders/basic.hpp"
#include "ogl/shaders/julia.hpp"
#include "ogl/shaders/particles.hpp"
#include "ogl/shaders/textured.hpp"
#include "ogl/shaders/texturedColorThreshold.hpp"

namespace Globals
{
	static std::unique_ptr<class Shaders> shaders;

	::Shaders::Programs::Basic& Shaders::basic()
	{
		return *basic_;
	}

	::Shaders::Programs::Julia& Shaders::julia()
	{
		return *julia_;
	}

	::Shaders::Programs::Particles& Shaders::particles()
	{
		return *particles_;
	}

	::Shaders::Programs::Textured& Shaders::textured()
	{
		return *textured_;
	}

	::Shaders::Programs::TexturedColorThreshold& Shaders::texturedColorThreshold()
	{
		return *texturedColorThreshold_;
	}

	void InitializeShaders()
	{
		shaders = std::make_unique<class Shaders>();
	}

	class Shaders& Shaders()
	{
		return *shaders;
	}
}
