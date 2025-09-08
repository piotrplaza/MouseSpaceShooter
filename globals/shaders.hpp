#pragma once

#include <memory>

namespace ShadersUtils::Programs
{
	struct BasicPhong;
	struct TexturedPhong;
	struct Basic;
	struct Julia;
	struct Billboards;
	struct Trails;;
	struct Textured;
	struct TexturedColorThreshold;
	struct Noise;
	struct Effects;
	struct TFParticles;
}

namespace Globals
{
	class ShadersHolder
	{
	public:
		ShadersUtils::Programs::BasicPhong& basicPhong();
		ShadersUtils::Programs::TexturedPhong& texturedPhong();
		ShadersUtils::Programs::Basic& basic();
		ShadersUtils::Programs::Julia& julia();
		ShadersUtils::Programs::Billboards& billboards();
		ShadersUtils::Programs::Trails& trails();
		ShadersUtils::Programs::Textured& textured();
		ShadersUtils::Programs::TexturedColorThreshold& texturedColorThreshold();
		ShadersUtils::Programs::Noise& noise();
		ShadersUtils::Programs::Effects& effects();
		ShadersUtils::Programs::TFParticles& tfParticles();

		void frameSetup();

	private:
		std::unique_ptr<ShadersUtils::Programs::BasicPhong> basicPhong_ = std::make_unique<ShadersUtils::Programs::BasicPhong>();
		std::unique_ptr<ShadersUtils::Programs::TexturedPhong> texturedPhong_ = std::make_unique<ShadersUtils::Programs::TexturedPhong>();
		std::unique_ptr<ShadersUtils::Programs::Basic> basic_ = std::make_unique<ShadersUtils::Programs::Basic>();
		std::unique_ptr<ShadersUtils::Programs::Julia> julia_ = std::make_unique<ShadersUtils::Programs::Julia>();
		std::unique_ptr<ShadersUtils::Programs::Billboards> billboards_ = std::make_unique<ShadersUtils::Programs::Billboards>();
		std::unique_ptr<ShadersUtils::Programs::Trails> trails_ = std::make_unique<ShadersUtils::Programs::Trails>();
		std::unique_ptr<ShadersUtils::Programs::Textured> textured_ = std::make_unique<ShadersUtils::Programs::Textured>();
		std::unique_ptr<ShadersUtils::Programs::TexturedColorThreshold> texturedColorThreshold_ = std::make_unique<ShadersUtils::Programs::TexturedColorThreshold>();
		std::unique_ptr<ShadersUtils::Programs::Noise> noise_ = std::make_unique<ShadersUtils::Programs::Noise>();
		std::unique_ptr<ShadersUtils::Programs::Effects> effects_ = std::make_unique<ShadersUtils::Programs::Effects>();
		std::unique_ptr<ShadersUtils::Programs::TFParticles> tfParticles_ = std::make_unique<ShadersUtils::Programs::TFParticles>();
	};

	void InitializeShaders();
	void DestroyShaders();

	class ShadersHolder& Shaders();
}
