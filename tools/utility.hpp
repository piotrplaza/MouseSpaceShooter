#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>

namespace Tools
{
	void RedirectIOToConsole(glm::ivec2 windowPos = {10, 10}, int maxConsoleLines = 500);

	void PrintWarning(const std::string& warning);
	void PrintError(const std::string& error);

	glm::vec2 GetNormalizedMousePosition();
	void SetMousePos(glm::ivec2 mousePos);
	glm::ivec2 GetMousePos();
	void SetMouseCursorVisibility(bool visibility);

	void RandomInit();
	float RandomFloat(float min, float max);
	int RandomInt(int min, int max);

	float ApplyDeadzone(float input, float deadzone = 0.3f);
	glm::vec2 ApplyDeadzone(glm::vec2 input, float deadzone = 0.3f, bool axesSeparation = false);

	template <class T>
	inline void HashCombine(std::size_t& seed, const T& v)
	{
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	inline glm::mat4 TextureTransform(glm::vec2 translate, float angle = 0.0f, glm::vec2 scale = { 1.0f, 1.0f })
	{
		return
			glm::translate(
				glm::rotate(
					glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / scale.x, 1.0f / scale.y, 1.0f)),
					angle, { 0.0f, 0.0f, -1.0f }),
				glm::vec3(-translate, 0.0f));
	}

	template <typename TextureComponent>
	inline glm::mat4 TextureTransform(const TextureComponent& textureComponent)
	{
		return
			glm::translate(
				glm::scale(glm::mat4(1.0f), glm::vec3(
					(textureComponent.preserveAspectRatio ? (float)textureComponent.loaded.size.y / textureComponent.loaded.size.x : 1.0f)
					* 1.0f / textureComponent.scale.x, 1.0f / textureComponent.scale.y, 1.0f)),
				glm::vec3(-textureComponent.translate, 0.0f));
	}

	template<class... Ts>
	struct Overloads : Ts... { using Ts::operator()...; };

	class ItToId
	{
	public:
		class It {
		public:
			using value_type = size_t;
			using difference_type = std::ptrdiff_t;

			It() : id(0) {}
			It(value_type id) : id(id) {}

			value_type operator*() const { return id; }
			It& operator++() { id++; return *this; }
			It operator++(int) { It tmp = *this; ++(*this); return tmp; }
			bool operator==(const It& other) const { return id == other.id; }
			bool operator!=(const It& other) const { return id != other.id; }

		private:
			value_type id;
		};

		ItToId(size_t size) :
			begin_(0),
			end_(size)
		{}

		ItToId(size_t begin, size_t end) :
			begin_(begin),
			end_(end)
		{
			assert(begin_ <= end_);
		}

		It begin() const
		{
			return It(begin_);
		}

		It end() const
		{
			return It(end_);
		}

	private:
		size_t begin_;
		size_t end_;
	};

	namespace StableRandom
	{
		struct Std1HashPolicy
		{
			static int Hash(int x)
			{
				x += (x << 10u);
				x ^= (x >> 6u);
				x += (x << 3u);
				x ^= (x >> 11u);
				x += (x << 15u);
				return x;
			}
		};

		struct Std2HashPolicy
		{
			static int Hash(int x)
			{
				int i = (x ^ 12345391u) * 2654435769u;
				i ^= (i << 6u) ^ (i >> 26u);
				i *= 2654435769u;
				i += (i << 5u) ^ (i >> 12u);
				return i;
			}
		};

		struct Std3HashPolicy
		{
			static int Hash(int x)
			{
				constexpr int prime = 16777619;
				constexpr int offset = (int)2166136261;

				return (offset ^ x) * prime;
			}
		};

		template <typename HashPolicy>
		struct Random : HashPolicy
		{
			using HashPolicy::Hash;

			static int Hash(glm::ivec2 v)
			{
				return Hash(v.x ^ Hash(v.y));
			}

			static int Hash(glm::ivec3 v)
			{
				return Hash(v.x ^ Hash(v.y) ^ Hash(v.z));
			}

			static int Hash(glm::ivec4 v)
			{
				return Hash(v.x ^ Hash(v.y) ^ Hash(v.z) ^ Hash(v.w));
			}

			static int Hash(int x, int seed)
			{
				return Hash(x ^ Hash(seed));
			}

			static int Hash(glm::ivec2 v, int seed)
			{
				return Hash(v.x ^ Hash(v.y) ^ Hash(seed));
			}

			static int Hash(glm::ivec3 v, int seed)
			{
				return Hash(v.x ^ Hash(v.y) ^ Hash(v.z) ^ Hash(seed));
			}

			static int Hash(glm::ivec4 v, int seed)
			{
				return Hash(v.x ^ Hash(v.y) ^ Hash(v.z) ^ Hash(v.w) ^ Hash(seed));
			}

			template <typename Seed>
			static int HashRange(int min, int max, Seed seed)
			{
				return (int)Hash(seed) % (max - min + 1) + min;
			}

			template <typename Seed>
			static glm::ivec2 HashRange(glm::ivec2 min, glm::ivec2 max, Seed seed)
			{
				int x = Hash(seed);
				int y = Hash(seed, x);

				return {
					x % (max.x - min.x + 1) + min.x,
					y % (max.y - min.y + 1) + min.y
				};
			}


			template <typename Seed>
			static glm::ivec3 HashRange(glm::ivec3 min, glm::ivec3 max, Seed seed)
			{
				unsigned x = Hash(seed);
				unsigned y = Hash(seed, x);
				unsigned z = Hash(seed, y);

				return {
					(int)(x % (max.x - min.x + 1) + min.x),
					(int)(y % (max.y - min.y + 1) + min.y),
					(int)(z % (max.z - min.z + 1) + min.z)
				};
			}

			template <typename Seed>
			static glm::ivec4 HashRange(glm::ivec4 min, glm::ivec4 max, Seed seed)
			{
				unsigned x = Hash(seed);
				unsigned y = Hash(seed, x);
				unsigned z = Hash(seed, y);
				unsigned w = Hash(seed, z);

				return {
					(int)(x % (max.x - min.x + 1) + min.x),
					(int)(y % (max.y - min.y + 1) + min.y),
					(int)(z % (max.z - min.z + 1) + min.z),
					(int)(w % (max.w - min.w + 1) + min.w)
				};
			}
		};

		using Std1Random = Random<Std1HashPolicy>;
		using Std2Random = Random<Std2HashPolicy>;
		using Std3Random = Random<Std3HashPolicy>;
	}
}
