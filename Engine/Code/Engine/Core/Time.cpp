//-----------------------------------------------------------------------------------------------
// Time.cpp
//	

//-----------------------------------------------------------------------------------------------
#include "Engine/Core/Time.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


//-----------------------------------------------------------------------------------------------
double InitializeTime( LARGE_INTEGER& out_initialTime )
{
	LARGE_INTEGER countsPerSecond;
	QueryPerformanceFrequency( &countsPerSecond );
	QueryPerformanceCounter( &out_initialTime );
	return( 1.0 / static_cast< double >( countsPerSecond.QuadPart ) );
}


//-----------------------------------------------------------------------------------------------
double GetCurrentTimeSeconds()
{
	static LARGE_INTEGER initialTime;
	static double secondsPerCount = InitializeTime( initialTime );
	LARGE_INTEGER currentCount;
	QueryPerformanceCounter( &currentCount );
	LONGLONG elapsedCountsSinceInitialTime = currentCount.QuadPart - initialTime.QuadPart;

	double currentSeconds = static_cast< double >( elapsedCountsSinceInitialTime ) * secondsPerCount;
	return currentSeconds;
}



uint64_t GetCurrentTimeInHPC()
{
	LARGE_INTEGER currentTimeInHPC;
	QueryPerformanceCounter( &currentTimeInHPC );
	return (uint64_t)currentTimeInHPC.QuadPart;
}


uint64_t QueryPerformanceFrequency()
{
	LARGE_INTEGER qpf;
	QueryPerformanceFrequency(&qpf);
	return (uint64_t)qpf.QuadPart;
}


double ConvertHPCtoSeconds(uint64_t timeInHPC)
{
	static double ONE_OVER_PERFORMANCE_FREQUENCY = 1.0f / (double)QueryPerformanceFrequency();
	return timeInHPC * ONE_OVER_PERFORMANCE_FREQUENCY;
}



uint64_t ConvertSecondsToHPC(double timeInSeconds)
{
	uint64_t timeInHPC = (uint64_t)(timeInSeconds * (double)QueryPerformanceFrequency());
	return timeInHPC;
}
