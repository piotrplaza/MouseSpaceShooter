#include "uniforms.hpp"

namespace
{
	constexpr bool invalidUniformsAllowed = false;
}

namespace Uniforms
{
	Uniform::Uniform() = default;

	Uniform::Uniform(Shaders::ProgramId programId, const std::string& uniformName) :
		programId(programId),
		uniformId(glGetUniformLocation(programId, uniformName.c_str()))
	{
		assert(isValidInternal());
	}

	bool Uniform::isValid() const
	{
		return uniformId != -1;
	}

	bool Uniform::isValidInternal() const
	{
		return invalidUniformsAllowed
			? true
			: isValid();
	}

	void Uniform1i::operator ()(int value)
	{
		assert(isValidInternal());
		glUseProgram_proxy(programId);
		glUniform1i(uniformId, value);
	}

	void Uniform2i::operator ()(glm::ivec2 value)
	{
		assert(isValidInternal());
		glUseProgram_proxy(programId);
		glUniform2i(uniformId, value.x, value.y);
	}

	void Uniform1b::operator ()(bool value)
	{
		assert(isValidInternal());
		glUseProgram_proxy(programId);
		glUniform1i(uniformId, value);
	}

	void Uniform1f::operator ()(float value)
	{
		assert(isValidInternal());
		glUseProgram_proxy(programId);
		glUniform1f(uniformId, value);
	}

	void Uniform2f::operator ()(glm::vec2 value)
	{
		assert(isValidInternal());
		glUseProgram_proxy(programId);
		glUniform2f(uniformId, value.x, value.y);
	}

	void Uniform3f::operator ()(glm::vec3 value)
	{
		assert(isValidInternal());
		glUseProgram_proxy(programId);
		glUniform3f(uniformId, value.x, value.y, value.z);
	}

	void Uniform4f::operator ()(glm::vec4 value)
	{
		assert(isValidInternal());
		glUseProgram_proxy(programId);
		glUniform4f(uniformId, value.x, value.y, value.z, value.w);
	}

	void UniformMat3f::operator ()(glm::mat3 value)
	{
		assert(isValidInternal());
		glUseProgram_proxy(programId);
		glUniformMatrix3fv(uniformId, 1, GL_FALSE, glm::value_ptr(value));
	}

	void UniformMat4f::operator ()(glm::mat4 value)
	{
		assert(isValidInternal());
		glUseProgram_proxy(programId);
		glUniformMatrix4fv(uniformId, 1, GL_FALSE, glm::value_ptr(value));
	}
}
