#include "uniformControllers.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "oglProxy.hpp"

namespace Uniforms
{
	UniformController::UniformController(Shaders::ProgramId programId, const std::string& uniformName) :
		programId(programId),
		uniformId(glGetUniformLocation(programId, uniformName.c_str()))
	{
	}

	bool UniformController::isValid() const
	{
		return uniformId != -1;
	}

	void UniformController1i::setValue(int value)
	{
		cache = value;
		glUseProgram_proxy(programId);
		glUniform1i(uniformId, value);
	}

	int UniformController1i::getValue() const
	{
		return cache;
	}

	void UniformController1f::setValue(float value)
	{
		cache = value;
		glUseProgram_proxy(programId);
		glUniform1f(uniformId, value);
	}

	float UniformController1f::getValue() const
	{
		return cache;
	}

	void UniformController2f::setValue(glm::vec2 value)
	{
		cache = value;
		glUseProgram_proxy(programId);
		glUniform2f(uniformId, value.x, value.y);
	}

	glm::vec2 UniformController2f::getValue() const
	{
		return cache;
	}

	void UniformController3f::setValue(glm::vec3 value)
	{
		cache = value;
		glUseProgram_proxy(programId);
		glUniform3f(uniformId, value.x, value.y, value.z);
	}

	glm::vec3 UniformController3f::getValue() const
	{
		return cache;
	}

	void UniformController4f::setValue(glm::vec4 value)
	{
		cache = value;
		glUseProgram_proxy(programId);
		glUniform4f(uniformId, value.x, value.y, value.z, value.w);
	}

	glm::vec4 UniformController4f::getValue() const
	{
		return cache;
	}

	void UniformControllerMat4f::setValue(glm::mat4 value)
	{
		cache = value;
		glUseProgram_proxy(programId);
		glUniformMatrix4fv(uniformId, 1, GL_FALSE, glm::value_ptr(value));
	}

	glm::mat4 UniformControllerMat4f::getValue() const
	{
		return cache;
	}

	UniformController1i GetUniformController1i(Shaders::ProgramId programId, const std::string& uniformName)
	{
		return { programId, uniformName };
	}

	UniformController1f GetUniformController1f(Shaders::ProgramId programId, const std::string& uniformName)
	{
		return { programId, uniformName };
	}

	UniformController2f GetUniformController2f(Shaders::ProgramId programId, const std::string& uniformName)
	{
		return { programId, uniformName };
	}

	UniformController3f GetUniformController3f(Shaders::ProgramId programId, const std::string& uniformName)
	{
		return { programId, uniformName };
	}

	UniformController4f GetUniformController4f(Shaders::ProgramId programId, const std::string& uniformName)
	{
		return { programId, uniformName };
	}

	UniformControllerMat4f GetUniformControllerMat4f(Shaders::ProgramId programId, const std::string& uniformName)
	{
		return { programId, uniformName };
	}
}
