#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

#include <functional>
#include <string>

namespace Details
{
	template<typename T>
	concept Callable = std::invocable<T>;
}

template <typename T>
class FType
{
public:
	FType() = default;
	FType(std::nullptr_t) {}
	FType(const T& value) : f([=]() { return value; }) {}
	FType(Details::Callable auto f) : f(std::move(f)) {}

	T operator()() const { return f(); }

	bool isLoaded() const { return (bool)f; }

private:
	std::function<T()> f;
};

using FInt = FType<int>;
using FUInt = FType<unsigned int>;
using FLong = FType<long>;
using FULong = FType<unsigned long>;
using FBool = FType<bool>;
using FChar = FType<char>;
using FFloat = FType<float>;
using FDouble = FType<double>;
using FVec2 = FType<glm::vec2>;
using FVec3 = FType<glm::vec3>;
using FVec4 = FType<glm::vec4>;
using FMat3 = FType<glm::mat3>;
using FMat4 = FType<glm::mat4>;
using FString = FType<std::string>;
