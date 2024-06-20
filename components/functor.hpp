#pragma once

#include "_componentBase.hpp"

#include <concepts>
#include <type_traits>

#include <commonTypes/fTypes.hpp>

namespace Components
{
	struct Functor : ComponentBase
	{
		Functor(FBool functor) :
			functor(std::move(functor))
		{
		}

		FBool functor;

		inline bool operator()()
		{
			return functor();
		}
	};
}
