#pragma once

#include "_componentBase.hpp"

#include <functional>
#include <concepts>
#include <type_traits>

namespace Components
{
	struct Functor : ComponentBase
	{
		Functor(std::function<bool()> functor) :
			functor(std::move(functor))
		{
		}

		std::function<bool()> functor;

		inline bool operator()()
		{
			return functor();
		}
	};
}
