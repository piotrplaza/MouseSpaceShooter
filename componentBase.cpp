#include "componentBase.hpp"

ComponentBase::ComponentBase() :
	id(ComponentIdGenerator::instance().acquire())
{
}
