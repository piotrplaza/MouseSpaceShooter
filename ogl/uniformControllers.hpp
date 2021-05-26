#pragma once

#include <GL/glew.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "shaders.hpp"

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

		void setValue(int value);
		int getValue() const;

	private:
		int cache{};
	};

	class UniformController2i : public UniformController
	{
	public:
		using UniformController::UniformController;

		void setValue(glm::ivec2 value);
		glm::ivec2 getValue() const;

	private:
		glm::ivec2 cache{};
	};

	class UniformController1f : public UniformController
	{
	public:
		using UniformController::UniformController;

		void setValue(float value);
		float getValue() const;

	private:
		float cache{};
	};

	class UniformController2f : public UniformController
	{
	public:
		using UniformController::UniformController;

		void setValue(glm::vec2 value);
		glm::vec2 getValue() const;

	private:
		glm::vec2 cache{};
	};

	class UniformController3f : public UniformController
	{
	public:
		using UniformController::UniformController;

		void setValue(glm::vec3 value);
		glm::vec3 getValue() const;

	private:
		glm::vec3 cache{};
	};

	class UniformController4f : public UniformController
	{
	public:
		using UniformController::UniformController;

		void setValue(glm::vec4 value);
		glm::vec4 getValue() const;

	private:
		glm::vec4 cache{};
	};

	class UniformControllerMat4f : public UniformController
	{
	public:
		using UniformController::UniformController;

		void setValue(glm::mat4 value);
		glm::mat4 getValue() const;

	private:
		glm::mat4 cache{};
	};
}
