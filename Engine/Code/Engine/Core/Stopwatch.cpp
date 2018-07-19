#include "Engine/Core/Stopwatch.hpp"

#include "Engine/Core/EngineCommon.h"

#include "Engine/Core/Clock.hpp"



Stopwatch::Stopwatch( Clock* owner )
{
	m_owner = owner;
	
	
	m_startHPC = GetMyCurrentTimeInHPC();
	m_intervalHPC = 0;
}



void Stopwatch::SetTimer( float seconds )
{
	uint64_t timeInHPC = ConvertSecondsToHPC(seconds);
	m_intervalHPC = timeInHPC;
}



uint64_t Stopwatch::GetElapsedTimeInHPC() const
{
	uint64_t currentTimeInHPC = GetMyCurrentTimeInHPC();
	uint64_t elapsedTimeInHPC = currentTimeInHPC - m_startHPC;
	return elapsedTimeInHPC;
}



float Stopwatch::GetElapsedTimeInSeconds() const
{
	return (float)ConvertHPCtoSeconds(GetElapsedTimeInHPC());
}



float Stopwatch::GetNormalizedElapsedTime() const
{
	if (m_intervalHPC == 0)
	{
		return -1.0f; // Dont do dumb things
	}

	float numberOfIntervalsCompleted = (float)(GetElapsedTimeInHPC() / m_intervalHPC);
	return numberOfIntervalsCompleted;
}



bool Stopwatch::HasElapsed() const
{
	bool hasElapsed = false;

	if (GetElapsedTimeInHPC() >= m_intervalHPC)
	{
		hasElapsed = true;
	}

	return hasElapsed;
}



void Stopwatch::Reset()
{
	m_startHPC = GetMyCurrentTimeInHPC();
}



bool Stopwatch::CheckAndReset()
{
	bool hasElapsed = HasElapsed();

	if (hasElapsed)
	{
		Reset();
	}

	return hasElapsed;
}



bool Stopwatch::Decrement()
{
	GUARANTEE_OR_DIE(m_intervalHPC != 0, "Stopwatch interval MUST BE INITIALIZED");

	bool hasElapsed = HasElapsed();

	if (hasElapsed)
	{
		m_startHPC += m_intervalHPC;
	}

	return hasElapsed;
}



unsigned int Stopwatch::DecrementAll()
{
	unsigned int numberOfDecrements = 0;

	while (Decrement())
	{
		++numberOfDecrements;
	}

	return numberOfDecrements;
}



uint64_t Stopwatch::GetMyCurrentTimeInHPC() const
{
	uint64_t currentTimeInHPC = 0;

	if (m_owner != nullptr)
	{
		currentTimeInHPC = m_owner->GetLifetimeInHPC();
	}
	else
	{
		currentTimeInHPC = GetCurrentTimeInHPC();
	}

	return currentTimeInHPC;
}
