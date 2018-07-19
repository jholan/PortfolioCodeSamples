#pragma once
#include "Game/Entity/Entity.hpp"

#include "Engine/Core/Stopwatch.hpp"

class PlayState;
class Mesh;
class Clock;



class PlayerTurret : public Entity
{
public:
	PlayerTurret(PlayState* owner);
	virtual ~PlayerTurret() override;

	virtual void	UpdateSelf(float deltaSeconds) override;
	void			Orient(float deltaSeconds);
	void			TryFire(float deltaSeconds);
	void			UpdateLazer();


private:
	Mesh*			m_mesh						= nullptr;
	Mesh*			m_barrelMesh				= nullptr;
	Entity*			m_lazer						= nullptr;

	FloatRange		m_xEulerRange				= FloatRange(-25.0f, 15.0f);
	float			m_rotationSpeed				= 5.0f;

	// Weapons
	Vector3			m_nose						= Vector3(0.0f, 0.0f, 1.0f);
	float			m_turnRateDegrees			= 90.0f;
	
	// Chaingun
	Stopwatch		m_fireStopwatch;
	float			m_bulletsPerSecond			= 10.0f;
	float			m_bulletSpreadHalfAngle		= 0.0f;
	
	// Buster shot
	bool			m_canFireBuster				= true;
	float			m_busterChargeTime			= 3.0f;
	Clock*			m_busterClock				= nullptr;
	Stopwatch*		m_busterChargeStopwatch		= nullptr;
	Stopwatch*		m_busterRechargeStopwatch	= nullptr;

	// Lazer
	RGBA			m_lazerColor				= RGBA(255,0,0);
};
