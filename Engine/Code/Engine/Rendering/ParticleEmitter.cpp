#include "Engine/Rendering/ParticleEmitter.hpp"

#include "Engine/Core/EngineCommon.h"

#include "Engine/Math/MathUtils.hpp"

#include "Engine/Rendering/Renderer.hpp"
#include "Engine/Rendering/Renderable.hpp"
#include "Engine/Rendering/Material.hpp"
#include "Engine/Rendering/MeshBuilder.hpp"
#include "Engine/Rendering/Camera.hpp"

#include "Engine/Core/Clock.hpp"
extern Clock* g_theGameClock;



Vector3 GetParticlePosition_DEFAULT()
{
	return Vector3::ZEROS;
}



Vector3 GetParticleVelocity_DEFUALT()
{
	return Vector3::ZEROS;
}



float GetParticleSize_DEFAULT()
{
	return GetRandomFloatInRange(0.1f, 0.2f);
}



float GetParticleMass_DEFUALT()
{
	return 1.0f;
}



float GetParticleLifespan_DEFAULT()
{
	return 1.0f;
}



Vector3 GetParticleForce_DEFAULT(float time)
{
	UNUSED(time);
	return Vector3(0.0f, -9.8f, 0.0f);
}



ParticleEmitter::ParticleEmitter()
	: m_spawnInterval(g_theGameClock)
{
	m_renderable = new Renderable();
	Material* particleMaterial = Material::FromShader("AdditiveParticle");
	particleMaterial->SetDiffuseTexture("Data/Images/ParticleSprite.png");
	m_renderable->SetMaterial(particleMaterial);

	m_spawnInterval.SetTimer(0.0f);

	// Callbacks
	m_GetParticlePosition_CB = GetParticlePosition_DEFAULT;
	m_GetParticleVelocity_CB = GetParticleVelocity_DEFUALT;
	m_GetParticleSize_CB	 = GetParticleSize_DEFAULT;
	m_GetParticleMass_CB	 = GetParticleMass_DEFUALT;
	m_GetParticleLifespan_CB = GetParticleLifespan_DEFAULT;
	m_GetParticleForce_CB	 = GetParticleForce_DEFAULT;
}



ParticleEmitter::~ParticleEmitter()
{
	delete m_renderable;
	m_renderable = nullptr;
}



Renderable* ParticleEmitter::GetRenderable() const
{
	return m_renderable;
}



void ParticleEmitter::UpdateRenderableForCamera(const Camera* camera)
{
	// free the old mesh
	if (m_mesh != nullptr)
	{
		delete m_mesh;
		m_renderable->SetMesh(nullptr);
	}


	// Generate our new mesh
	MeshBuilder builder;
	builder.Initialize(Renderer::PRIMITIVE_TRIANGLES, false);

	// Choose up/right depending on the space of the particles
	Vector3 right;
	Vector3 up;
	GetRightAndUpForEmitterSpace(right, up, camera);

	for (int i = 0; i < (int)m_particles.size(); ++i) 
	{
		// Add a billboarded quad for each particle
		Particle& particle = m_particles[i]; 
		builder.AddBillboardedQuad(particle.m_position, right, up, particle.m_size);
		//DebugDraw_WireSphere(0.0f, particle.m_position, particle.m_size, RGBA(0,255,0), RGBA(0,255,0), DEBUG_RENDER_USE_DEPTH);
		//DebugDraw_WireAABB3(0.0f, particle.m_position, Vector3(particle.m_size, particle.m_size, particle.m_size), RGBA(0,255,0), RGBA(0,255,0), DEBUG_RENDER_USE_DEPTH);
	}

	builder.Finalize();
	m_mesh = builder.CreateMesh(Vertex_3DPCU::GetLayout());
	m_renderable->SetMesh(m_mesh);
}



void ParticleEmitter::SetTransform(const Matrix4& transform)
{
	m_transform = transform;

	// If we are a local space particle emitter we need to update the localToWorld matrix of the renderable
	if (!m_emitterSpace)
	{
		m_renderable->SetModelMatrix(transform);
	}
}



void ParticleEmitter::SetParticleSpawnPositionCallback(ParticlePosition_CB cb, Space space)
{
	m_GetParticlePosition_CB = cb;
	m_positionSpace = space;
}



void ParticleEmitter::SetParticleSpawnVelocityCallback(ParticleVelocity_CB cb, Space space)
{
	m_GetParticleVelocity_CB = cb;
	m_velocitySpace = space;
}



void ParticleEmitter::SetParticleSpawnSizeCallback(ParticleSize_CB cb)
{
	m_GetParticleSize_CB = cb;
}



void ParticleEmitter::SetParticleSpawnMassCallback(ParticleMass_CB cb)
{
	m_GetParticleMass_CB = cb;
}



void ParticleEmitter::SetParticleSpawnLifespanCallback(ParticleLifespan_CB cb)
{
	m_GetParticleLifespan_CB = cb;
}



void ParticleEmitter::SetParticleForceCallback(ParticleForce_CB cb, Space space)
{
	m_GetParticleForce_CB = cb;
	m_forceSpace = space;
}


#include "Engine/Rendering/DebugRender.hpp"
void ParticleEmitter::Update(float deltaSeconds)
{
	// Spawn new particles 
	if (m_spawnOverTime && !PendingDestruction())
	{
		unsigned int numParticlesToSpawn = m_spawnInterval.DecrementAll(); 
		SpawnParticles(numParticlesToSpawn); 
	}

	for (unsigned int i = 0; i < m_particles.size(); ++i)
	{
		Particle& particle = m_particles[i]; 

		if (i == 0)
		{
			DebugDraw_Log(0.0f, "pos = (%f, %f, %f)", particle.m_position.x, particle.m_position.y, particle.m_position.z);
		}


		particle.m_force = GetParticleForceSpaceAware(particle.m_lifetime);
		particle.Update(deltaSeconds); 

		// Remove all particles pending destruction
		if (particle.CanBeDestroyed())
		{
			unsigned int numParticles = (unsigned int)m_particles.size();
			m_particles[i] = m_particles[numParticles - 1];
			m_particles.erase(m_particles.begin() + numParticles - 1);			
		} 
	}
}



void ParticleEmitter::SpawnParticle()
{
	Matrix4 localToWorld = m_transform;

	Particle particle; 
	particle.m_position = GetParticlePositionSpaceAware(); 
	particle.m_velocity = GetParticleVelocitySpaceAware();
	particle.m_size		= m_GetParticleSize_CB();  
	particle.m_mass		= m_GetParticleMass_CB();
	particle.m_lifespan = m_GetParticleLifespan_CB();

	m_particles.push_back(particle);
}



void ParticleEmitter::SpawnParticles(unsigned int count)
{
	// Short Circuit
	if (PendingDestruction())
	{
		return;
	}

	for (unsigned int i = 0; i < count; ++i)
	{
		SpawnParticle(); 
	}
}



void ParticleEmitter::SetSpawnRate(float particlesPerSecond)
{
	if (particlesPerSecond == 0.0f) 
	{
		m_spawnOverTime = false; 
	} 
	else 
	{
		m_spawnOverTime = true; 
		m_spawnInterval.SetTimer(1.0f / particlesPerSecond); 
	}
}



void ParticleEmitter::RequestDestroy()
{
	m_pendingDestroy = true;
}



bool ParticleEmitter::PendingDestruction() const
{
	return m_pendingDestroy;
}



bool ParticleEmitter::CanBeDestroyed() const
{
	bool canBeDestroyed = false;

	if (m_pendingDestroy && m_particles.size() == 0)
	{
		canBeDestroyed = true;
	}

	return canBeDestroyed;
}



void ParticleEmitter::SetEmitterSpace(Space space)
{
	m_emitterSpace = space;

	if (space == SPACE_WORLD)
	{
		m_renderable->SetModelMatrix(Matrix4());
	}
	else if (space == SPACE_LOCAL)
	{
		m_renderable->SetModelMatrix(m_transform);
	}
}



Space ParticleEmitter::GetEmitterSpace() const
{
	return m_emitterSpace;
}



Vector3 ParticleEmitter::GetParticlePositionSpaceAware() const
{
	Vector3 position;

	if (m_positionSpace != m_emitterSpace)
	{
		if (m_positionSpace == SPACE_LOCAL && m_emitterSpace == SPACE_WORLD)
		{
			position = (m_transform * Vector4(m_GetParticlePosition_CB(), 1.0f)).XYZ();
		}
		else if (m_positionSpace == SPACE_WORLD && m_emitterSpace == SPACE_LOCAL)
		{
			Matrix4 worldToLocal = m_transform;
			worldToLocal.Invert();
			position = (worldToLocal * Vector4(m_GetParticlePosition_CB(), 1.0f)).XYZ();
		}
		else
		{
			GUARANTEE_OR_DIE(false, "UNEXPECTED PARTICLE SPACE COMBINATION!");
		}
	}
	else
	{
		position = m_GetParticlePosition_CB();
	}

	return position;
}



Vector3 ParticleEmitter::GetParticleVelocitySpaceAware() const
{
	Vector3 velocity;
	
	if (m_velocitySpace != m_emitterSpace)
	{
		if (m_velocitySpace == SPACE_LOCAL && m_emitterSpace == SPACE_WORLD)
		{
			velocity = (m_transform * Vector4(m_GetParticleVelocity_CB(), 0.0f)).XYZ();
		}
		else if (m_velocitySpace == SPACE_WORLD && m_emitterSpace == SPACE_LOCAL)
		{
			Matrix4 worldToLocal = m_transform;
			worldToLocal.Invert();
			velocity = (worldToLocal * Vector4(m_GetParticleVelocity_CB(), 0.0f)).XYZ();
		}
		else
		{
			GUARANTEE_OR_DIE(false, "UNEXPECTED PARTICLE SPACE COMBINATION!");
		}
	}
	else
	{
		velocity = m_GetParticleVelocity_CB();
	}

	return velocity;
}



Vector3 ParticleEmitter::GetParticleForceSpaceAware(float time) const
{
	Vector3 force;

	if (m_forceSpace != m_emitterSpace)
	{
		if (m_forceSpace == SPACE_LOCAL && m_emitterSpace == SPACE_WORLD)
		{
			force = (m_transform * Vector4(m_GetParticleForce_CB(time), 0.0f)).XYZ(); 
		}
		else if (m_forceSpace == SPACE_WORLD && m_emitterSpace == SPACE_LOCAL)
		{
			Matrix4 worldToLocal = m_transform;
			worldToLocal.Invert();
			force = (worldToLocal * Vector4(m_GetParticleForce_CB(time), 0.0f)).XYZ();
		}
		else
		{
			GUARANTEE_OR_DIE(false, "UNEXPECTED PARTICLE SPACE COMBINATION!");
		}
	}
	else
	{
		force = m_GetParticleForce_CB(time);
	}
	
	return force;
}



void ParticleEmitter::GetRightAndUpForEmitterSpace(Vector3& right, Vector3& up, const Camera* camera)
{
	right = camera->GetRight();
	up = camera->GetUp();
	if (m_emitterSpace == SPACE_LOCAL)
	{
		Matrix4 worldToLocal = m_transform;
		worldToLocal.Invert();

		right = (worldToLocal * Vector4(camera->GetRight(), 0.0f)).XYZ(); 
		up = (worldToLocal * Vector4(camera->GetUp(), 0.0f)).XYZ(); 
	}
}
