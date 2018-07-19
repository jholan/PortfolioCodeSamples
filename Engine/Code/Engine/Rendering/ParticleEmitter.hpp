#pragma once

#include <vector>

#include "Engine/Core/EngineCommon.h"

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/Math/IntRange.hpp"

#include "Engine/Core/Stopwatch.hpp"

class Renderable;
class Mesh;
class Camera;

// Callbacks to create generate spawn parameters
typedef Vector3 (*ParticlePosition_CB)();
typedef Vector3 (*ParticleVelocity_CB)();
typedef float	(*ParticleSize_CB)();
typedef float	(*ParticleMass_CB)();
typedef float	(*ParticleLifespan_CB)();

// Called every particle update
typedef Vector3 (*ParticleForce_CB)(float time);


#include "Engine/Rendering/DebugRender.hpp"
class Particle
{
public:
	Vector3 m_position	= Vector3::ZEROS;
	Vector3 m_velocity	= Vector3::ZEROS;
	Vector3 m_force		= Vector3::ZEROS;

	float	m_size		= 1.0f; 
	float	m_mass		= 1.0f; 

	float   m_lifetime	= 0.0f;
	float	m_lifespan	= 1.0f;

	void Update(float deltaSeconds)
	{
		Vector3 acceleration = m_force / m_mass;
		m_velocity += (acceleration * deltaSeconds);
		m_position += (m_velocity * deltaSeconds);

		m_force = Vector3::ZEROS;

		m_lifetime += deltaSeconds;
	}

	float GetNormalizedAge() const
	{
		float normalizedAge = 1.0f;

		if (m_lifespan != 0.0f)
		{
			normalizedAge = m_lifetime / m_lifespan;
		}

		return normalizedAge;
	}

	bool CanBeDestroyed() const
	{
		bool canDestroy = false;

		if (GetNormalizedAge() > 1.0f)
		{
			canDestroy = true;
		}

		return canDestroy;
	}
};



class ParticleEmitter
{
public:
public:
	ParticleEmitter();
	~ParticleEmitter();

	Renderable* GetRenderable() const;
	void		UpdateRenderableForCamera(const Camera* camera);

	void		SetTransform(const Matrix4& transform);

	void		SetParticleSpawnPositionCallback(ParticlePosition_CB cb, Space space = SPACE_LOCAL);
	void		SetParticleSpawnVelocityCallback(ParticleVelocity_CB cb, Space space = SPACE_LOCAL);
	void		SetParticleSpawnSizeCallback(ParticleSize_CB cb);
	void		SetParticleSpawnMassCallback(ParticleMass_CB cb);
	void		SetParticleSpawnLifespanCallback(ParticleLifespan_CB cb);

	void		SetParticleForceCallback(ParticleForce_CB cb, Space space = SPACE_WORLD);

	void		Update(float deltaSeconds); 
	void		SpawnParticle(); 
	void		SpawnParticles(unsigned int count); 

	void		SetSpawnRate(float particlesPerSecond); 

	void		RequestDestroy();
	bool		PendingDestruction() const;
	bool		CanBeDestroyed() const;

	void		SetEmitterSpace(Space space);
	Space		GetEmitterSpace() const;


private:
	Vector3		GetParticlePositionSpaceAware() const;
	Vector3		GetParticleVelocitySpaceAware() const;
	Vector3		GetParticleForceSpaceAware(float time) const;
	void		GetRightAndUpForEmitterSpace(Vector3& right, Vector3& up, const Camera* camera);


	Matrix4					m_transform				 = Matrix4(); 
	Renderable*				m_renderable			 = nullptr;
	Mesh*					m_mesh					 = nullptr;
	Space					m_emitterSpace			 = SPACE_WORLD;
	std::vector<Particle>	m_particles; 

	// Particle spawning
	ParticlePosition_CB		m_GetParticlePosition_CB = nullptr;
	ParticleVelocity_CB		m_GetParticleVelocity_CB = nullptr;
	ParticleSize_CB			m_GetParticleSize_CB	 = nullptr;
	ParticleMass_CB			m_GetParticleMass_CB	 = nullptr;
	ParticleLifespan_CB		m_GetParticleLifespan_CB = nullptr;

	// Transforms
	Space					m_positionSpace			 = SPACE_LOCAL;
	Space					m_velocitySpace			 = SPACE_LOCAL;
	Space					m_forceSpace			 = SPACE_LOCAL;

	// Called every particle update
	ParticleForce_CB		m_GetParticleForce_CB	 = nullptr;


	// For spawning particles over time
	bool					m_spawnOverTime			 = false; 
	Stopwatch				m_spawnInterval;

	// Fore spawning in bursts
	IntRange				m_burstParticleCount	 = IntRange(0); 

	bool					m_pendingDestroy		 = false;
};