#pragma once

#include <functional>

#include <componentBase.hpp>

namespace Components
{
	struct Functor : ComponentBase
	{
		Functor(std::function<void()> functor) :
			functor(functor)
		{
		}

		std::function<void()> functor;

		inline void operator()()
		{
			functor();
		}
	};
}
