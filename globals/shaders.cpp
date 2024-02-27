#include "shaders.hpp"

#include "ogl/shaders/basicPhong.hpp"
#include "ogl/shaders/texturedPhong.hpp"
#include "ogl/shaders/basic.hpp"
#include "ogl/shaders/julia.hpp"
#include "ogl/shaders/particles.hpp"
#include "ogl/shaders/textured.hpp"
#include "ogl/shaders/texturedColorThreshold.hpp"

namespace Globals
{
	static std::unique_ptr<ShadersHolder> shadersHolder;

	Shaders::Programs::BasicPhong& ShadersHolder::basicPhong()
	{
		return *basicPhong_;
	}

	Shaders::Programs::TexturedPhong& ShadersHolder::texturedPhong()
	{
		return *texturedPhong_;
	}

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

	void ShadersHolder::frameSetup()
	{
		auto setup = [](auto& program) {
			if (program.frameSetupF)
				program.frameSetupF(program);
		};

		setup(basicPhong());
		setup(texturedPhong());
		setup(basic());
		setup(julia());
		setup(particles());
		setup(textured());
		setup(texturedColorThreshold());
	}

	void InitializeShaders()
	{
		shadersHolder = std::make_unique<ShadersHolder>();
	}

	void DestroyShaders()
	{
		shadersHolder.reset();
	}

	ShadersHolder& Shaders()
	{
		return *shadersHolder;
	}
}
