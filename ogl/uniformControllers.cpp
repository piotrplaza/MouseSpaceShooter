#include "uniformControllers.hpp"

#include <glm/gtc/type_ptr.hpp>

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

	void UniformController1i::operator ()(int value)
	{
		assert(isValid());
		glUseProgram_proxy(programId);
		glUniform1i(uniformId, value);
	}

	void UniformController2i::operator ()(glm::ivec2 value)
	{
		assert(isValid());
		glUseProgram_proxy(programId);
		glUniform2i(uniformId, value.x, value.y);
	}

	void UniformController1b::operator ()(bool value)
	{
		assert(isValid());
		glUseProgram_proxy(programId);
		glUniform1i(uniformId, value);
	}

	void UniformController1f::operator ()(float value)
	{
		assert(isValid());
		glUseProgram_proxy(programId);
		glUniform1f(uniformId, value);
	}

	void UniformController2f::operator ()(glm::vec2 value)
	{
		assert(isValid());
		glUseProgram_proxy(programId);
		glUniform2f(uniformId, value.x, value.y);
	}

	void UniformController3f::operator ()(glm::vec3 value)
	{
		assert(isValid());
		glUseProgram_proxy(programId);
		glUniform3f(uniformId, value.x, value.y, value.z);
	}

	void UniformController4f::operator ()(glm::vec4 value)
	{
		assert(isValid());
		glUseProgram_proxy(programId);
		glUniform4f(uniformId, value.x, value.y, value.z, value.w);
	}

	void UniformControllerMat4f::operator ()(glm::mat4 value)
	{
		assert(isValid());
		glUseProgram_proxy(programId);
		glUniformMatrix4fv(uniformId, 1, GL_FALSE, glm::value_ptr(value));
	}
}
