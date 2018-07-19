#pragma once

#include <string>

#include "Engine/Math/AABB2D.hpp"

class SpriteSheet;
class Texture;
class SpriteAnimDefinition;



class SpriteAnim
{
public:
	// Composition
	SpriteAnim(const SpriteAnimDefinition* defintion);
	~SpriteAnim();

	// Core
	void Update( float deltaSeconds );

	// Rendering
	const Texture*  GetTexture() const;
	AABB2D			GetCurrentTexCoords() const;	// Based on the current elapsed time
	
	// Name
	const std::string& GetName() const;

	// Play State
	bool  IsFinished() const;		
	bool  IsPlaying() const;	
	void  Reset();						// Rewinds to time 0 and starts (re)playing
	
	// Pause
	void  SetPauseState(bool state);
	void  Pause();						// Starts unpaused (playing) by default
	void  Resume();						// Resume after pausing

	// Elapsed Time
	float GetSecondsElapsed() const;	
	float GetFractionElapsed() const;
	
	// Time Remaining
	float GetSecondsRemaining() const;
	float GetFractionRemaining() const;
	
	// Scrub
	void  SetSecondsElapsed( float secondsElapsed );	     // Jump to specific time
	void  SetFractionElapsed( float fractionElapsed );       // e.g. 0.33f for one-third in

	// Duration
	float GetDurationSeconds() const;	

	bool IsAutoOrient() const;


private:
	const SpriteAnimDefinition* m_definition; 
	float m_elapsedSeconds = 0.0f;
	bool  m_isPlaying	   = true;
	bool  m_isFinished	   = false;
};
