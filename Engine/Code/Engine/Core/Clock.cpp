#include "Engine/Core/Clock.hpp"

#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/Time.hpp"

extern Clock* g_theMasterClock;


void Clock::Initialize(Clock* parent)
{
	m_parent = parent;
	if (m_parent != nullptr)
	{
		parent->m_children.push_back(this);
	}

	Reset();
}



void Clock::Destroy()
{
	// Re parent all children to the master clock
	for (size_t i = 0; i < m_children.size(); ++i)
	{
		m_children[i]->m_parent = g_theMasterClock;

	}
	m_parent = nullptr;
	m_children.clear();
}



void Clock::Reset()
{
	m_previousTimeInHPC = GetCurrentTimeInHPC();
	m_frameTime = ClockTime();
	m_totalTime = ClockTime();
}


#include "Engine/Profiler/ProfileScope.hpp"
void Clock::BeginFrame()
{
	uint64_t currentTimeInHPC = GetCurrentTimeInHPC();
	uint64_t elapsedTimeInHPC = currentTimeInHPC - m_previousTimeInHPC;
	Step(elapsedTimeInHPC);

	m_previousTimeInHPC = currentTimeInHPC;
}



void Clock::EndFrame()
{

}



void Clock::Step(uint64_t elapsedTimeInHPC, bool stepWhilePaused)
{
	uint64_t scaledElapsedTime;
	if(IsPaused() && !stepWhilePaused)
	{
		scaledElapsedTime = 0;
	}
	else
	{
		scaledElapsedTime = (uint64_t)(elapsedTimeInHPC * m_timescale);
	}

	m_frameTime.m_inHPC = scaledElapsedTime;
	m_frameTime.m_inSeconds = (float)ConvertHPCtoSeconds(scaledElapsedTime);

	m_totalTime.m_inHPC += scaledElapsedTime;
	m_totalTime.m_inSeconds = (float)ConvertHPCtoSeconds(m_totalTime.m_inHPC);

	for (size_t i = 0; i < m_children.size(); ++i)
	{
		m_children[i]->Step(scaledElapsedTime);
	}
}



void Clock::StepInSeconds(float elapsedTimeInSeconds, bool stepWhilePaused)
{
	uint64_t elapsedTimeInHPC = ConvertSecondsToHPC(elapsedTimeInSeconds);
	Step(elapsedTimeInHPC, stepWhilePaused);
}



bool Clock::IsPaused() const
{
	return m_isPaused;
}



void Clock::SetPauseState(bool newPauseState)
{
	m_isPaused = newPauseState;
}



void Clock::Pause()
{
	SetPauseState(true);
}



void Clock::Unpause()
{
	SetPauseState(false);
}



void  Clock::SetTimescale(float newTimescale)
{
	m_timescale = newTimescale;
}



float Clock::GetTimescale() const
{
	return m_timescale;
}



float Clock::GetDeltaSeconds() const
{
	return m_frameTime.m_inSeconds;
}



uint64_t Clock::GetDeltaSecondsInHPC() const
{
	return m_frameTime.m_inHPC;
}



float Clock::GetLifetimeInSeconds() const
{
	return m_totalTime.m_inSeconds;
}



uint64_t Clock::GetLifetimeInHPC() const
{
	return m_totalTime.m_inHPC;
}