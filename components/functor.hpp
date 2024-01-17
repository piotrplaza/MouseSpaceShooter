#pragma once

#include "_componentBase.hpp"

#include <functional>
#include <concepts>
#include <type_traits>

template<typename Func>
concept BoolCallable = std::is_same_v<bool, std::invoke_result_t<Func>>;

template<typename Func>
concept VoidCallable = std::is_same_v<void, std::invoke_result_t<Func>>;

namespace Components
{
	struct Functor : ComponentBase
	{
		Functor(BoolCallable auto functor) :
			functor(std::move(functor))
		{
		}

		Functor(VoidCallable auto functor) :
			functor([functor = std::move(functor)]() {functor(); return true; })
		{
		}

		std::function<bool()> functor;

		inline bool operator()()
		{
			return functor();
		}
	};
}
