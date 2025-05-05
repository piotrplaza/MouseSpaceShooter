#include "uniformsUtils.hpp"

#include "shaders/programBase.hpp"

namespace
{
	constexpr bool invalidUniformsAllowed = false;
	constexpr bool redundantInitAllowed = false;
}

namespace UniformsUtils
{
	Uniform::Uniform() = default;

	Uniform::Uniform(ShadersUtils::ProgramId programId, const std::string& uniformName, bool checkpointCapturing) :
		programId(programId),
		uniformId(glGetUniformLocation(programId, uniformName.c_str())),
		checkpointCapturing(checkpointCapturing)
	{
		assert(isValidInternal());
		const bool firstInit = ShadersUtils::AccessorBase::programIdsToPrograms.at(programId)->uniforms.insert({ uniformId, this }).second;
		assert(firstInit || redundantInitAllowed);
	}

	void Uniform::reset(ShadersUtils::ProgramId programId, const std::string& uniformName)
	{
		this->programId = programId;
		uniformId = glGetUniformLocation(programId, uniformName.c_str());
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
		glProxyUseProgram(programId);
		this->value = value;
		glUniform1i(uniformId, this->value);
	}

	int Uniform1i::getValue() const
	{
		assert(isValidInternal());
		return value;
	}

	std::function<void()> Uniform1i::getCheckpoint()
	{
		return createCheckpoint(this, value);
	}

	void Uniform2i::operator ()(glm::ivec2 value)
	{
		assert(isValidInternal());
		glProxyUseProgram(programId);
		this->value = value;
		glUniform2i(uniformId, this->value.x, this->value.y);
	}

	const glm::ivec2& Uniform2i::getValue() const
	{
		assert(isValidInternal());
		return value;
	}

	std::function<void()> Uniform2i::getCheckpoint()
	{
		return createCheckpoint(this, value);
	}

	void Uniform1b::operator ()(bool value)
	{
		assert(isValidInternal());
		glProxyUseProgram(programId);
		this->value = value;
		glUniform1i(uniformId, this->value);
	}

	bool Uniform1b::getValue() const
	{
		assert(isValidInternal());
		return value;
	}

	std::function<void()> Uniform1b::getCheckpoint()
	{
		return createCheckpoint(this, value);
	}

	void Uniform1f::operator ()(float value)
	{
		assert(isValidInternal());
		glProxyUseProgram(programId);
		this->value = value;
		glUniform1f(uniformId, this->value);
	}

	float Uniform1f::getValue() const
	{
		assert(isValidInternal());
		return value;
	}

	std::function<void()> Uniform1f::getCheckpoint()
	{
		return createCheckpoint(this, value);
	}

	void Uniform2f::operator ()(glm::vec2 value)
	{
		assert(isValidInternal());
		glProxyUseProgram(programId);
		this->value = value;
		glUniform2f(uniformId, this->value.x, this->value.y);
	}

	const glm::vec2& Uniform2f::getValue() const
	{
		assert(isValidInternal());
		return value;
	}

	std::function<void()> Uniform2f::getCheckpoint()
	{
		return createCheckpoint(this, value);
	}

	void Uniform3f::operator ()(glm::vec3 value)
	{
		assert(isValidInternal());
		glProxyUseProgram(programId);
		this->value = value;
		glUniform3f(uniformId, this->value.x, this->value.y, this->value.z);
	}

	const glm::vec3& Uniform3f::getValue() const
	{
		assert(isValidInternal());
		return value;
	}

	std::function<void()> Uniform3f::getCheckpoint()
	{
		return createCheckpoint(this, value);
	}

	void Uniform4f::operator ()(glm::vec4 value)
	{
		assert(isValidInternal());
		glProxyUseProgram(programId);
		this->value = value;
		glUniform4f(uniformId, this->value.x, this->value.y, this->value.z, this->value.w);
	}

	const glm::vec4& Uniform4f::getValue() const
	{
		assert(isValidInternal());
		return value;
	}

	std::function<void()> Uniform4f::getCheckpoint()
	{
		return createCheckpoint(this, value);
	}

	void UniformMat3f::operator ()(glm::mat3 value)
	{
		assert(isValidInternal());
		glProxyUseProgram(programId);
		this->value = value;
		glUniformMatrix3fv(uniformId, 1, GL_FALSE, glm::value_ptr(this->value));
	}

	const glm::mat3& UniformMat3f::getValue() const
	{
		assert(isValidInternal());
		return value;
	}

	std::function<void()> UniformMat3f::getCheckpoint()
	{
		return createCheckpoint(this, value);
	}

	void UniformMat4f::operator ()(glm::mat4 value)
	{
		assert(isValidInternal());
		glProxyUseProgram(programId);
		this->value = value;
		glUniformMatrix4fv(uniformId, 1, GL_FALSE, glm::value_ptr(this->value));
	}

	const glm::mat4& UniformMat4f::getValue() const
	{
		assert(isValidInternal());
		return value;
	}

	std::function<void()> UniformMat4f::getCheckpoint()
	{
		return createCheckpoint(this, value);
	}
}
