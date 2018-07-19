#include "Engine/Profiler/ScopedProfileLogger.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"



ScopedProfileLogger::ScopedProfileLogger(const char* name)
{
	m_startHPC = GetCurrentTimeInHPC();
	m_name = name;
}



ScopedProfileLogger::~ScopedProfileLogger()
{
	uint64_t endHPC = GetCurrentTimeInHPC();
	uint64_t elapsedHPC = endHPC - m_startHPC;
	float elapsedTimeSeconds = (float)ConvertHPCtoSeconds(elapsedHPC);

	DebuggerPrintf("%s Elapsed Time = %fms\n", m_name, elapsedTimeSeconds * 1000.0f);
}