#include "Engine/Profiler/ProfileScope.hpp"

#include "Engine/Profiler/Profiler.hpp"



ProfileScope::ProfileScope(const char* name)
{
	Profiler_Push(name);
}



ProfileScope::~ProfileScope()
{
	Profiler_Pop();
}