#pragma once

struct ComponentBase
{
	using Id = unsigned long;

	static void ReleaseId(Id id);

	ComponentBase();

	const Id id;
	bool changed = true;
};
