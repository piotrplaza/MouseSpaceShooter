#include "uniformControllers.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "oglProxy.hpp"

namespace Uniforms
{
	UniformController::UniformController() = default;

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
		assert(isValid());
		cache = value;
		glUseProgram_proxy(programId);
		glUniform1i(uniformId, value);
	}

	int UniformController1i::getValue() const
	{
		assert(isValid());
		return cache;
	}

	void UniformController2i::setValue(glm::ivec2 value)
	{
		assert(isValid());
		cache = value;
		glUseProgram_proxy(programId);
		glUniform2i(uniformId, value.x, value.y);
	}

	glm::ivec2 UniformController2i::getValue() const
	{
		assert(isValid());
		return cache;
	}

	void UniformController1f::setValue(float value)
	{
		assert(isValid());
		cache = value;
		glUseProgram_proxy(programId);
		glUniform1f(uniformId, value);
	}

	float UniformController1f::getValue() const
	{
		assert(isValid());
		return cache;
	}

	void UniformController2f::setValue(glm::vec2 value)
	{
		assert(isValid());
		cache = value;
		glUseProgram_proxy(programId);
		glUniform2f(uniformId, value.x, value.y);
	}

	glm::vec2 UniformController2f::getValue() const
	{
		assert(isValid());
		return cache;
	}

	void UniformController3f::setValue(glm::vec3 value)
	{
		assert(isValid());
		cache = value;
		glUseProgram_proxy(programId);
		glUniform3f(uniformId, value.x, value.y, value.z);
	}

	glm::vec3 UniformController3f::getValue() const
	{
		assert(isValid());
		return cache;
	}

	void UniformController4f::setValue(glm::vec4 value)
	{
		assert(isValid());
		cache = value;
		glUseProgram_proxy(programId);
		glUniform4f(uniformId, value.x, value.y, value.z, value.w);
	}

	glm::vec4 UniformController4f::getValue() const
	{
		assert(isValid());
		return cache;
	}

	void UniformControllerMat4f::setValue(glm::mat4 value)
	{
		assert(isValid());
		cache = value;
		glUseProgram_proxy(programId);
		glUniformMatrix4fv(uniformId, 1, GL_FALSE, glm::value_ptr(value));
	}

	glm::mat4 UniformControllerMat4f::getValue() const
	{
		assert(isValid());
		return cache;
	}
}
