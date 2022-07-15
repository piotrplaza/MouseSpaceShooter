#pragma once

#include "_componentBase.hpp"

#include <functional>

namespace Components
{
	struct Functor : ComponentBase
	{
		Functor() = default;

		Functor(std::function<void()> functor) :
			functor(std::move(functor))
		{
		}

		std::function<void()> functor;

		inline void operator()()
		{
			functor();
		}
	};
}
