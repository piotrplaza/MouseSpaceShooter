#pragma once

#include <string>
#include <map>

#include <GL/glew.h>

namespace Shaders
{
	using ShaderId = GLuint;
	using ProgramId = GLuint;
	using ShaderType = GLenum;
	using AttribLocation = GLuint;

	struct ShadersBase
	{
		virtual void attach(ProgramId program) const = 0;
		virtual void detach(ProgramId program) const = 0;
	};

	struct VertexFragmentShaders : ShadersBase
	{
		VertexFragmentShaders(ShaderId vertexShader, ShaderId fragmentShader);

		ShaderId vertexShader;
		ShaderId fragmentShader;

		void attach(ProgramId program) const override;
		void detach(ProgramId program) const override;
	};

	struct VertexGeometryFragmentShaders : ShadersBase
	{
		VertexGeometryFragmentShaders(ShaderId vertexShader, ShaderId geometryShader, ShaderId fragmentShader);

		ShaderId vertexShader;
		ShaderId geometryShader;
		ShaderId fragmentShader;

		void attach(ProgramId program) const override;
		void detach(ProgramId program) const override;
	};

	ShaderId CompileShader(const std::string& path, const ShaderType shaderType);
	VertexFragmentShaders CompileShaders(const std::string& vsPath, const std::string& fsPath);
	VertexGeometryFragmentShaders CompileShaders(const std::string& vsPath, const std::string& gsPath, const std::string& fsPath);
	ProgramId LinkProgram(const ShadersBase& shaders, const std::map<AttribLocation, std::string>& attribLocationsToNames);
}
