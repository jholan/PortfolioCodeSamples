#pragma once

#include <vector>

#include "Engine/Core/EngineCommon.h"

#include "Engine/Core/Transform.hpp"
#include "Game/Components/Tags.hpp"

class PlayState;

class Renderable;
class Light;
class ParticleEmitter;
class HealthPool;



class Entity
{
public:
	Entity(PlayState* owner);
	virtual ~Entity();


	// Ownership
	void							SetOwner(PlayState* owner);
	PlayState*						GetOwner();
	const PlayState* const			GetOwner() const;


	// Hierarchy
	bool							HasParent() const;
	Entity*							GetParent();
	const Entity* const				GetParent() const;

	unsigned int					GetNumChildren() const;
	Entity*							GetChild(unsigned int index) const;
	void							AddChild(Entity* child);
	void							RemoveChild(Entity* child);


	// Components
	Transform&						GetTransform();
	const Transform&				GetTransform() const;

	Tags&							GetTags();
	const Tags&						GetTags() const;

	bool							HasRenderable() const;
	void							AddRenderable();
	Renderable*						GetRenderable();
	const Renderable* const			GetRenderable() const;

	bool							HasLight() const;
	void							AddLight();
	Light*							GetLight();
	const Light* const				GetLight() const;

	bool							HasParticleEmitter() const;
	void							AddParticleEmitter();
	ParticleEmitter*				GetParticleEmitter();
	const ParticleEmitter* const	GetParticleEmitter() const;

	bool							HasHealthPool() const;
	void							AddHealthPool(float max = 100.0f, float current = -1.0f);
	HealthPool*						GetHealthPool();
	const HealthPool* const			GetHealthPool() const;


	// Updating
	void							Update(float deltaSeconds);
	void							UpdateComponents(float deltaSeconds);
	virtual void					UpdateSelf(float deltaSeconds) { UNUSED(deltaSeconds); }; // Any smart entities should override this
	

	// Collision
	virtual void					OnCollision(Entity* other) { UNUSED(other); };
	float							GetCollisionRadius() const;
	void							SetCollisionRadius(float radius);


	// Destruction
	virtual bool					CanBeDestroyed();
	void							QueueForDestruction();


	// Lifetime
	float							GetAgeInSeconds() const;


private:
	PlayState*				m_owner = nullptr;

	Entity*					m_parent = nullptr;
	std::vector<Entity*>	m_children;

	Transform				m_transform;
	Tags					m_tags;
	Renderable*				m_renderable		= nullptr;
	Light*					m_light				= nullptr;
	ParticleEmitter*		m_particleEmitter	= nullptr;
	HealthPool*				m_healthPool		= nullptr;

	float					m_collisionRadius	= 1.0f;

	bool					m_wantsToDie = false;

	float					m_age = 0.0f;
};