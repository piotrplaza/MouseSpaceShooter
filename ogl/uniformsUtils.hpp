#pragma once

#include "shadersUtils.hpp"

#include "oglProxy.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <functional>

namespace ShadersUtils
{
	struct AccessorBase;
}

namespace UniformsUtils
{
	class Uniform
	{
	public:
		Uniform();
		Uniform(ShadersUtils::ProgramId programId, const std::string& uniformName, bool checkpointCapturing = true);

		bool isValid() const;
		void reset(ShadersUtils::ProgramId programId, const std::string& uniformName);

		virtual std::function<void()> getCheckpoint() = 0;

	protected:
		std::function<void()> createCheckpoint(auto this_, auto value)
		{
			if (!checkpointCapturing)
				return []() {};
			return [=]() { this_->operator()(value); };
		}

		bool isValidInternal() const;

		ShadersUtils::ProgramId programId = 0;
		GLint uniformId = -1;
		bool checkpointCapturing = true;
	};

	class Uniform1i : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(int value);

		int getValue() const;

		std::function<void()> getCheckpoint() override;

	private:
		int value{};
	};

	template <unsigned Size>
	class Uniform1iv : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(int value)
		{
			assert(isValidInternal());
			glProxyUseProgram(programId);
			values.fill(value);
			glUniform1iv(uniformId, Size, values.data());
		}

		void operator ()(unsigned index, int value)
		{
			assert(isValidInternal());
			assert(index < Size);
			glProxyUseProgram(programId);
			glUniform1i(uniformId + index, value);
		}

		void operator ()(const std::array<int, Size>& values)
		{
			assert(isValidInternal());
			assert(values.size() == Size);
			glProxyUseProgram(programId);
			this->values = values;
			glUniform1iv(uniformId, Size, this->values.data());
		}

		const std::array<int, Size>& getValues() const
		{
			assert(isValidInternal());
			return values;
		}

		std::function<void()> getCheckpoint() override
		{
			return createCheckpoint(this, values);
		}

	private:
		std::array<int, Size> values{};
	};

	class Uniform2i : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(glm::ivec2 value);
		
		const glm::ivec2& getValue() const;

		std::function<void()> getCheckpoint() override;

	private:
		glm::ivec2 value{};
	};

	template <unsigned Size>
	class Uniform2iv : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(glm::ivec2 value)
		{
			assert(isValidInternal());
			glProxyUseProgram(programId);
			values.fill(value);
			glUniform2iv(uniformId, Size, &values[0][0]);
		}

		void operator ()(unsigned index, glm::ivec2 value)
		{
			assert(isValidInternal());
			assert(index < Size);
			glProxyUseProgram(programId);
			glUniform2i(uniformId + index, value.x, value.y);
		}

		void operator ()(const std::array<glm::ivec2, Size>& values)
		{
			assert(isValidInternal());
			assert(values.size() == Size);
			glProxyUseProgram(programId);
			this->values = values;
			glUniform2iv(uniformId, Size, &this->values[0][0]);
		}

		const std::array<glm::ivec2, Size>& getValues() const
		{
			assert(isValidInternal());
			return values;
		}

		std::function<void()> getCheckpoint() override
		{
			return createCheckpoint(this, values);
		}

	private:
		std::array<glm::ivec2, Size> values{};
	};

	class Uniform1b : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(bool value);
		
		bool getValue() const;

		std::function<void()> getCheckpoint() override;

	private:
		bool value{};
	};

	template <unsigned Size>
	class Uniform1bv : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(bool value)
		{
			assert(isValidInternal());
			glProxyUseProgram(programId);
			values.fill(value);
			glUniform1iv(uniformId, Size, values.data());
		}

		void operator ()(unsigned index, bool value)
		{
			assert(isValidInternal());
			assert(index < Size);
			glProxyUseProgram(programId);
			glUniform1i(uniformId + index, value);
		}

		void operator ()(const std::array<int, Size>& values)
		{
			assert(isValidInternal());
			assert(values.size() == Size);
			glProxyUseProgram(programId);
			this->values = values;
			glUniform1iv(uniformId, Size, this->values.data());
		}

		const std::array<int, Size>& getValues() const
		{
			assert(isValidInternal());
			return values;
		}

		std::function<void()> getCheckpoint() override
		{
			return createCheckpoint(this, values);
		}

	private:
		std::array<int, Size> values{};
	};

	class Uniform1f : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(float value);
		
		float getValue() const;

		std::function<void()> getCheckpoint() override;

	private:
		float value{};
	};

	template <unsigned Size>
	class Uniform1fv : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(float value)
		{
			assert(isValidInternal());
			glProxyUseProgram(programId);
			values.fill(value);
			glUniform1fv(uniformId, Size, values.data());
		}

		void operator ()(unsigned index, float value)
		{
			assert(isValidInternal());
			assert(index < Size);
			glProxyUseProgram(programId);
			glUniform1f(uniformId + index, value);
		}

		void operator ()(const std::array<float, Size>& values)
		{
			assert(isValidInternal());
			assert(values.size() == Size);
			glProxyUseProgram(programId);
			this->values = values;
			glUniform1fv(uniformId, Size, this->values.data());
		}

		const std::array<float, Size>& getValues() const
		{
			assert(isValidInternal());
			return values;
		}

		std::function<void()> getCheckpoint() override
		{
			return createCheckpoint(this, values);
		}

	private:
		std::array<float, Size> values{};
	};

	class Uniform2f : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(glm::vec2 value);
		
		const glm::vec2& getValue() const;

		std::function<void()> getCheckpoint() override;

	private:
		glm::vec2 value{};
	};

	template <unsigned Size>
	class Uniform2fv : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(glm::vec2 value)
		{
			assert(isValidInternal());
			glProxyUseProgram(programId);
			values.fill(value);
			glUniform2fv(uniformId, Size, &values[0][0]);
		}

		void operator ()(unsigned index, glm::vec2 value)
		{
			assert(isValidInternal());
			assert(index < Size);
			glProxyUseProgram(programId);
			glUniform2f(uniformId + index, value.x, value.y);
		}

		void operator ()(const std::array<glm::vec2, Size>& values)
		{
			assert(isValidInternal());
			assert(values.size() == Size);
			glProxyUseProgram(programId);
			this->values = values;
			glUniform2fv(uniformId, Size, &this->values[0][0]);
		}

		const std::array<glm::vec2, Size>& getValues() const
		{
			assert(isValidInternal());
			return values;
		}

		std::function<void()> getCheckpoint() override
		{
			return createCheckpoint(this, values);
		}

	private:
		std::array<glm::vec2, Size> values{};
	};

	class Uniform3f : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(glm::vec3 value);

		const glm::vec3& getValue() const;

		std::function<void()> getCheckpoint() override;

	private:
		glm::vec3 value{};
	};

	template <unsigned Size>
	class Uniform3fv : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(glm::vec3 value)
		{
			assert(isValidInternal());
			glProxyUseProgram(programId);
			values.fill(value);
			glUniform3fv(uniformId, Size, &values[0][0]);
		}

		void operator ()(unsigned index, glm::vec3 value)
		{
			assert(isValidInternal());
			assert(index < Size);
			glProxyUseProgram(programId);
			glUniform3f(uniformId + index, value.x, value.y, value.z);
		}

		void operator ()(const std::array<glm::vec3, Size>& values)
		{
			assert(isValidInternal());
			assert(values.size() == Size);
			glProxyUseProgram(programId);
			this->values = values;
			glUniform3fv(uniformId, Size, &this->values[0][0]);
		}

		const std::array<glm::vec3, Size>& getValues() const
		{
			assert(isValidInternal());
			return values;
		}

		std::function<void()> getCheckpoint() override
		{
			return createCheckpoint(this, values);
		}

	private:
		std::array<glm::vec3, Size> values{};
	};

	class Uniform4f : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(glm::vec4 value);

		const glm::vec4& getValue() const;

		std::function<void()> getCheckpoint() override;

	private:
		glm::vec4 value{};
	};

	template <unsigned Size>
	class Uniform4fv : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(glm::vec4 value)
		{
			assert(isValidInternal());
			glProxyUseProgram(programId);
			values.fill(value);
			glUniform4fv(uniformId, Size, &values[0][0]);
		}

		void operator ()(unsigned index, glm::vec4 value)
		{
			assert(isValidInternal());
			assert(index < Size);
			glProxyUseProgram(programId);
			glUniform4f(uniformId + index, value.x, value.y, value.z, value.t);
		}

		void operator ()(const std::array<glm::vec4, Size>& values)
		{
			assert(isValidInternal());
			assert(values.size() == Size);
			glProxyUseProgram(programId);
			this->values = values;
			glUniform4fv(uniformId, Size, &this->values[0][0]);
		}

		const std::array<glm::vec4, Size>& getValues() const
		{
			assert(isValidInternal());
			return values;
		}

		std::function<void()> getCheckpoint() override  
		{  
			return createCheckpoint(this, values);
		}

	private:
		std::array<glm::vec4, Size> values{};
	};

	class UniformMat3f : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(glm::mat3 value);

		const glm::mat3& getValue() const;

		std::function<void()> getCheckpoint() override;

	private:
		glm::mat3 value{};
	};

	template <unsigned Size>
	class UniformMat3fv : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(glm::mat3 value)
		{
			assert(isValidInternal());
			glProxyUseProgram(programId);
			values.fill(value);
			glUniformMatrix3fv(uniformId, Size, GL_FALSE, glm::value_ptr(values[0]));
		}

		void operator ()(unsigned index, glm::mat3 value)
		{
			assert(isValidInternal());
			assert(index < Size);
			glProxyUseProgram(programId);
			glUniformMatrix3fv(uniformId + index, 1, GL_FALSE, glm::value_ptr(value));
		}

		void operator ()(const std::array<glm::mat3, Size>& values)
		{
			assert(isValidInternal());
			assert(values.size() == Size);
			glProxyUseProgram(programId);
			this->values = values;
			glUniformMatrix3fv(uniformId, Size, GL_FALSE, &this->values[0][0][0]);
		}

		const std::array<glm::mat3, Size>& getValues() const
		{
			assert(isValidInternal());
			return values;
		}

		std::function<void()> getCheckpoint() override
		{
			return createCheckpoint(this, values);
		}

	private:
		std::array<glm::mat3, Size> values{};
	};

	class UniformMat4f : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(glm::mat4 value);

		const glm::mat4& getValue() const;

		std::function<void()> getCheckpoint() override;

	private:
		glm::mat4 value{};
	};

	template <unsigned Size>
	class UniformMat4fv : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(glm::mat4 value)
		{
			assert(isValidInternal());
			glProxyUseProgram(programId);
			values.fill(value);
			glUniformMatrix4fv(uniformId, Size, GL_FALSE, glm::value_ptr(values[0]));
		}

		void operator ()(unsigned index, glm::mat4 value)
		{
			assert(isValidInternal());
			assert(index < Size);
			glProxyUseProgram(programId);
			glUniformMatrix4fv(uniformId + index, 1, GL_FALSE, glm::value_ptr(value));
		}

		void operator ()(const std::array<glm::mat4, Size>& values)
		{
			assert(isValidInternal());
			assert(values.size() == Size);
			glProxyUseProgram(programId);
			this->values = values;
			glUniformMatrix4fv(uniformId, Size, GL_FALSE, &this->values[0][0][0]);
		}

		const std::array<glm::mat4, Size>& getValues() const
		{
			assert(isValidInternal());
			return values;
		}

		std::function<void()> getCheckpoint() override
		{
			return createCheckpoint(this, values);
		}

	private:
		std::array<glm::mat4, Size> values{};
	};
}
