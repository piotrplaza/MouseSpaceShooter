#include "shaders.hpp"

#include <fstream>
#include <sstream>
#include <unordered_map>

using namespace std::string_literals;

namespace Shaders
{
	static const std::unordered_map< ShaderType, std::string> shaderTypesToNames =
	{
		{GL_VERTEX_SHADER, "vertex shader"},
		{GL_GEOMETRY_SHADER, "geometry shader"},
		{GL_FRAGMENT_SHADER, "fragment shader"}
	};

	VertexFragmentShaders::VertexFragmentShaders(ShaderId vertexShader, ShaderId fragmentShader):
		vertexShader(vertexShader),
		fragmentShader(fragmentShader)
	{
	}

	void VertexFragmentShaders::attach(ProgramId program) const
	{
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
	}

	void VertexFragmentShaders::detach(ProgramId program) const
	{
		glDetachShader(program, vertexShader);
		glDetachShader(program, fragmentShader);
	}

	VertexGeometryFragmentShaders::VertexGeometryFragmentShaders(ShaderId vertexShader, ShaderId geometryShader, ShaderId fragmentShader):
		vertexShader(vertexShader),
		geometryShader(geometryShader),
		fragmentShader(fragmentShader)
	{
	}

	void VertexGeometryFragmentShaders::attach(ProgramId program) const
	{
		glAttachShader(program, vertexShader);
		glAttachShader(program, geometryShader);
		glAttachShader(program, fragmentShader);
	}

	void VertexGeometryFragmentShaders::detach(ProgramId program) const
	{
		glDetachShader(program, vertexShader);
		glDetachShader(program, geometryShader);
		glDetachShader(program, fragmentShader);
	}

	ShaderId CompileShader(const std::string& path, ShaderType shaderType)
	{
		std::ifstream file(path);
		if (file)
		{
			std::string source = [&]()
			{
				std::ostringstream ss;
				ss << file.rdbuf();
				return ss.str();
			}();

			ShaderId shader = glCreateShader(shaderType);
			{
				const auto cSource = source.c_str();
				GLint compileStatus;

				glShaderSource(shader, 1, &cSource, nullptr);
				glCompileShader(shader);
				glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

				if (compileStatus != GL_TRUE)
				{
					const unsigned int maxLogLength = 1024;
					char shaderInfoLog[maxLogLength];
					glGetShaderInfoLog(shader, maxLogLength, nullptr, shaderInfoLog);
					glDeleteShader(shader);
					throw std::runtime_error("Unable to compile " + shaderTypesToNames.at(shaderType)
						+ " \"" + path + "\".\n" + shaderInfoLog);
				}
			}
			return shader;
		}
		else
		{
			throw std::runtime_error("Unable to open \"" + path + "\".");
		}
	}

	VertexFragmentShaders CompileShaders(const std::string& vsPath, const std::string& fsPath)
	{
		return { CompileShader(vsPath, GL_VERTEX_SHADER), CompileShader(fsPath, GL_FRAGMENT_SHADER) };
	}

	VertexGeometryFragmentShaders CompileShaders(const std::string& vsPath, const std::string& gsPath, const std::string& fsPath)
	{
		return { CompileShader(vsPath, GL_VERTEX_SHADER), CompileShader(gsPath, GL_GEOMETRY_SHADER), CompileShader(fsPath, GL_FRAGMENT_SHADER) };
	}

	ProgramId LinkProgram(const ShadersBase& shaders, const std::map<AttribLocation, std::string>& attribLocationsToNames)
	{
		ProgramId program = glCreateProgram();

		shaders.attach(program);

		for (const auto& attribLocationAndName : attribLocationsToNames)
		{
			glBindAttribLocation(program, attribLocationAndName.first, attribLocationAndName.second.c_str());
		}
		
		GLint linkStatus;

		glLinkProgram(program);
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE)
		{
			const unsigned int maxLogLength = 1024;
			char programInfoLog[maxLogLength];
			glGetProgramInfoLog(program, maxLogLength, nullptr, programInfoLog);
			glDeleteProgram(program);
			throw std::runtime_error("Unable to link program.\n"s + programInfoLog);
		}

		shaders.detach(program);

		return program;
	}
}
