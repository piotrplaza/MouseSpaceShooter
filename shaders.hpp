#pragma once

#include <string>
#include <map>

#include <GL/glew.h>

namespace Shaders
{
	using ShaderId = GLuint;
	using ProgramId = GLuint;
	using UniformId = GLint;
	using ShaderType = GLenum;
	using AttribLocation = GLuint;

	struct VertexAndFragmentShader
	{
		ShaderId vertexShader;
		ShaderId fragmentShader;
	};

	ShaderId CompileShader(const std::string & path, const ShaderType shaderType);
	VertexAndFragmentShader CompileShaders(const std::string & vsPath, const std::string & fsPath);
	ProgramId LinkProgram(const VertexAndFragmentShader & shaders,
		const std::map<AttribLocation, std::string> & attribLocationsToNames);
}
