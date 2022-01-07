#pragma once

#include "shaders.hpp"

#include "oglProxy.hpp"

#include <GL/glew.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <array>

namespace Uniforms
{
	class UniformController
	{
	public:
		UniformController();
		UniformController(Shaders::ProgramId programId, const std::string& uniformName);

		bool isValid() const;

	protected:
		Shaders::ProgramId programId = 0;
		GLint uniformId = -1;
	};

	class UniformController1i : public UniformController
	{
	public:
		using UniformController::UniformController;

		void operator ()(int value);
	};

	template <unsigned Size>
	class UniformController1iv : public UniformController
	{
	public:
		using UniformController::UniformController;

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

	class UniformController2i : public UniformController
	{
	public:
		using UniformController::UniformController;

		void operator ()(glm::ivec2 value);
	};

	class UniformController1b : public UniformController
	{
	public:
		using UniformController::UniformController;

		void operator ()(bool value);
	};

	class UniformController1f : public UniformController
	{
	public:
		using UniformController::UniformController;

		void operator ()(float value);
	};

	class UniformController2f : public UniformController
	{
	public:
		using UniformController::UniformController;

		void operator ()(glm::vec2 value);
	};

	template <unsigned Size>
	class UniformController2fv : public UniformController
	{
	public:
		using UniformController::UniformController;

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

	class UniformController3f : public UniformController
	{
	public:
		using UniformController::UniformController;

		void operator ()(glm::vec3 value);
	};

	class UniformController4f : public UniformController
	{
	public:
		using UniformController::UniformController;

		void operator ()(glm::vec4 value);
	};

	class UniformControllerMat4f : public UniformController
	{
	public:
		using UniformController::UniformController;

		void operator ()(glm::mat4 value);
	};
}
