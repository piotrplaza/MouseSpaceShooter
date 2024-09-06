#include "shaders.hpp"

#include "ogl/shaders/basicPhong.hpp"
#include "ogl/shaders/texturedPhong.hpp"
#include "ogl/shaders/basic.hpp"
#include "ogl/shaders/julia.hpp"
#include "ogl/shaders/particles.hpp"
#include "ogl/shaders/textured.hpp"
#include "ogl/shaders/texturedColorThreshold.hpp"
#include "ogl/shaders/noise.hpp"
#include "ogl/shaders/tfOrbitingParticles.hpp"

namespace Globals
{
	static std::unique_ptr<ShadersHolder> shadersHolder;

	ShadersUtils::Programs::BasicPhong& ShadersHolder::basicPhong()
	{
		return *basicPhong_;
	}

	ShadersUtils::Programs::TexturedPhong& ShadersHolder::texturedPhong()
	{
		return *texturedPhong_;
	}

	ShadersUtils::Programs::Basic& ShadersHolder::basic()
	{
		return *basic_;
	}

	ShadersUtils::Programs::Julia& ShadersHolder::julia()
	{
		return *julia_;
	}

	ShadersUtils::Programs::Particles& ShadersHolder::particles()
	{
		return *particles_;
	}

	ShadersUtils::Programs::Textured& ShadersHolder::textured()
	{
		return *textured_;
	}

	ShadersUtils::Programs::TexturedColorThreshold& ShadersHolder::texturedColorThreshold()
	{
		return *texturedColorThreshold_;
	}

	ShadersUtils::Programs::Noise& ShadersHolder::noise()
	{
		return *noise_;
	}

	ShadersUtils::Programs::TFOrbitingParticles& ShadersHolder::tfOrbitingParticles()
	{
		return *tfOrbitingParticles_;
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
		setup(noise());
		setup(tfOrbitingParticles());
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
