#pragma once

#include "shaders.hpp"

#include "oglProxy.hpp"

#include <GL/glew.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>

namespace Uniforms
{
	class Uniform
	{
	public:
		Uniform();
		Uniform(Shaders::ProgramId programId, const std::string& uniformName);

		bool isValid() const;

	protected:
		Shaders::ProgramId programId = 0;
		GLint uniformId = -1;
	};

	class Uniform1i : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(int value);
	};

	template <unsigned Size>
	class Uniform1iv : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(int value)
		{
			assert(isValid());
			glUseProgram_proxy(programId);
			std::array<int, Size> values;
			values.fill(value);
			glUniform1iv(uniformId, Size, values.data());
		}

		void operator ()(unsigned index, int value)
		{
			assert(isValid());
			assert(index < Size);
			glUseProgram_proxy(programId);
			glUniform1i(uniformId + index, value);
		}

		void operator ()(const std::array<int, Size>& values)
		{
			assert(isValid());
			assert(values.size() == Size);
			glUseProgram_proxy(programId);
			glUniform1iv(uniformId, Size, values.data());
		}
	};

	class Uniform2i : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(glm::ivec2 value);
	};

	class Uniform1b : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(bool value);
	};

	class Uniform1f : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(float value);
	};

	class Uniform2f : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(glm::vec2 value);
	};

	template <unsigned Size>
	class Uniform2fv : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(glm::vec2 value)
		{
			assert(isValid());
			glUseProgram_proxy(programId);
			std::array<glm::vec2, Size> values;
			values.fill(value);
			glUniform2fv(uniformId, Size, &values[0][0]);
		}

		void operator ()(unsigned index, glm::vec2 value)
		{
			assert(isValid());
			assert(index < Size);
			glUseProgram_proxy(programId);
			glUniform2f(uniformId + index, value.x, value.y);
		}

		void operator ()(const std::array<glm::vec2, Size>& values)
		{
			assert(isValid());
			assert(values.size() == Size);
			glUseProgram_proxy(programId);
			glUniform2fv(uniformId, Size, values.data());
		}
	};

	class Uniform3f : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(glm::vec3 value);
	};

	class Uniform4f : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(glm::vec4 value);
	};

	class UniformMat4f : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(glm::mat4 value);
	};

	template <unsigned Size>
	class UniformMat4fv : public Uniform
	{
	public:
		using Uniform::Uniform;

		void operator ()(glm::mat4 value)
		{
			assert(isValid());
			glUseProgram_proxy(programId);
			std::array<glm::mat4, Size> values;
			values.fill(value);
			glUniformMatrix4fv(uniformId, Size, GL_FALSE, glm::value_ptr(values[0]));
		}

		void operator ()(unsigned index, glm::mat4 value)
		{
			assert(isValid());
			assert(index < Size);
			glUseProgram_proxy(programId);
			glUniformMatrix4fv(uniformId + index, 1, GL_FALSE, glm::value_ptr(value));
		}

		void operator ()(const std::array<glm::mat4, Size>& values)
		{
			assert(isValid());
			assert(values.size() == Size);
			glUseProgram_proxy(programId);
			glUniformMatrix4fv(uniformId, Size, GL_FALSE, values.data());
		}
	};
}
