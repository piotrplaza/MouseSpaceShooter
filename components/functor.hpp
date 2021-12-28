#pragma once

#include <functional>

#include <componentBase.hpp>

namespace Components
{
	struct Functor : ComponentBase
	{
		using ComponentBase::ComponentBase;

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
