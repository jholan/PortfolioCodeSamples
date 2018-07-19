#pragma once

#include <vector>

struct ClockTime
{
	uint64_t m_inHPC	 = 0;
	float    m_inSeconds = 0.0f;
};


class Clock
{
public:
	// Composition
	Clock()  {}; 
	~Clock() {};

	void		Initialize(Clock* parent = nullptr);
	void		Destroy();
	void		Reset();

	// Updating
	void		BeginFrame();
	void		EndFrame();
	void		Step(uint64_t elapsedTimeInHPC, bool stepWhilePaused = false);
	void		StepInSeconds(float elapsedTimeInSeconds, bool stepWhilePaused = false);

	// Pausing
	bool		IsPaused() const;
	void		SetPauseState(bool newPauseState);
	void		Pause();
	void		Unpause();

	// Timescale
	void		SetTimescale(float newTimescale = 1.0f);
	float		GetTimescale() const;

	// Time Queries
	float		GetDeltaSeconds() const;
	uint64_t	GetDeltaSecondsInHPC() const;

	float		GetLifetimeInSeconds() const;
	uint64_t	GetLifetimeInHPC() const;



private:
	// Clock Nesting
	Clock*				 m_parent	= nullptr;
	std::vector<Clock*>  m_children;

	ClockTime m_frameTime;
	ClockTime m_totalTime;
	uint64_t  m_previousTimeInHPC; // Only used by master clocks

	bool  m_isPaused  = false;
	float m_timescale = 1.0f;
};