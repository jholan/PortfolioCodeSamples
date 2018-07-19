#pragma once

#include "Engine/Core/Time.hpp"

class Clock;


class Stopwatch
{
public:
	Stopwatch( Clock* owner ); 

	// change our reference clock. 
	// should maintain elapsed time 
	//void set_clock( Clock *ref_clock ); 

	// sets the "interval time" for
	// this stopwtach.
	// takes seconds to be easy to use
	// but under the hood it will use hpc
	void SetTimer( float seconds ); 

	// if we really just want to use it as a timer
	// this returns how long since the last reset
	uint64_t GetElapsedTimeInHPC() const;
	float GetElapsedTimeInSeconds() const;

	// elapsed time divided by the interval
	float GetNormalizedElapsedTime() const; 

	// returns true if the interval has elapsed;
	bool HasElapsed() const; 

	// resets the interval
	// (get_elapsed_time() should return 0.0f);
	void Reset(); 

	// checks the interval, if it has elapsed, will
	// return true and reset.  If not, will just return false (no reset)
	bool CheckAndReset(); 

	// returns has_elapsed(), and if so
	// will subtract one interval from 
	// the elapsed time (but maintaining 
	// any extra time we have accrued)
	bool Decrement(); 

	// like decrement, but returns how many 
	// times we have elapsed our timer
	// so say our timer was 0.5 seconds, 
	// and get_elapsed_time() was 2.2s.  
	// decrement_all() would return 4, and 
	// update itself so that get_elapsed_time()
	// would return 0.2s;
	unsigned int DecrementAll(); 

	// pause and resume [extra/optional]
	// void pause(); 
	// void resume()

private:
	uint64_t GetMyCurrentTimeInHPC() const;

	// clock to use as reference
	Clock *m_owner; 

	// reference times; 
	// notice using these member
	// variables, our stop-wtach doesn't need
	// an "update", like it would if we
	// were accumulating deltas
	uint64_t m_startHPC; 
	uint64_t m_intervalHPC; 
};