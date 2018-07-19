#include "Game/Entity/Entity.hpp"

#include "Game/GameState/PlayState.hpp"
#include "Engine/Rendering/RenderScene.hpp"

#include "Engine/Rendering/Renderable.hpp"
#include "Engine/Rendering/Light.hpp"
#include "Engine/Rendering/ParticleEmitter.hpp"
#include "Game/Components/HealthPool.hpp"



Entity::Entity(PlayState* owner)
{
	SetOwner(owner); 
}



Entity::~Entity()
{
	if (m_renderable != nullptr)
	{
		GetOwner()->GetRenderScene()->RemoveRenderable(m_renderable);
		delete m_renderable;
		m_renderable = nullptr;
	}

	if (m_light != nullptr)
	{
		GetOwner()->GetRenderScene()->RemoveLight(m_light);
		delete m_light;
		m_light = nullptr;
	}
	
	if (m_particleEmitter != nullptr)
	{
		GetOwner()->GetRenderScene()->RemoveParticleEmitter(m_particleEmitter);
		delete m_particleEmitter;
		m_particleEmitter = nullptr;
	}

	for (int i = 0; i < (int)m_children.size(); ++i)
	{
		delete m_children[i];
	}
	m_children.clear();
}



void Entity::SetOwner(PlayState* owner)
{
	m_owner = owner;
}



PlayState* Entity::GetOwner()
{
	return m_owner;
}



const PlayState* const Entity::GetOwner() const
{
	return m_owner;
}



bool Entity::HasParent() const
{
	bool hasParent = false;

	if (m_parent != nullptr)
	{
		hasParent = true;
	}

	return hasParent;
}



Entity*	Entity::GetParent()
{
	return m_parent;
}



const Entity* const Entity::GetParent() const
{
	return m_parent;
}



unsigned int Entity::GetNumChildren() const
{
	return (unsigned int)m_children.size();
}



Entity*	Entity::GetChild(unsigned int index) const
{
	Entity* child = nullptr;

	if (index < GetNumChildren())
	{
		child = m_children[index];
	}

	return child;
}



void Entity::AddChild(Entity* child)
{
	// Short Circuit
	if (child == nullptr)
	{
		return;
	}

	if (child->HasParent())
	{
		child->GetParent()->RemoveChild(child);
	}

	m_children.push_back(child);
	child->GetTransform().SetParent(&GetTransform());
}



void Entity::RemoveChild(Entity* child)
{
	// Try to find the child
	bool isPresent = false;
	int childIndex = -1;
	for (int i = 0; i < (int)m_children.size(); ++i)
	{
		if (m_children[i] == child)
		{
			isPresent = true;
			childIndex = i;
			break;
		}
	}


	// If its present remove it from our list of children and null out its parent
	if (isPresent)
	{
		m_children[childIndex]->m_parent = nullptr;
		m_children[childIndex]->GetTransform().SetParent(nullptr);
		m_children.erase(m_children.begin() + childIndex);
	}
}



Transform& Entity::GetTransform()
{
	return m_transform;
}



const Transform& Entity::GetTransform() const
{
	return m_transform;
}



Tags& Entity::GetTags()
{
	return m_tags;
}



const Tags&	Entity::GetTags() const
{
	return m_tags;
}



bool Entity::HasRenderable() const
{
	bool exists = false;

	if (m_renderable != nullptr)
	{
		exists = true;
	}

	return exists;
}



void Entity::AddRenderable()
{
	if (m_renderable == nullptr)
	{
		m_renderable = new Renderable();
		GetOwner()->GetRenderScene()->AddRenderable(m_renderable);
	}
}



Renderable* Entity::GetRenderable()
{
	return m_renderable;
}



const Renderable* const Entity::GetRenderable() const
{
	return m_renderable;
}



bool Entity::HasLight() const
{
	bool exists = false;

	if (m_light != nullptr)
	{
		exists = true;
	}

	return exists;
}



void Entity::AddLight()
{
	if (m_light == nullptr)
	{
		m_light = new Light();
		GetOwner()->GetRenderScene()->AddLight(m_light);
	}
}



Light* Entity::GetLight()
{
	return m_light;
}



const Light* const Entity::GetLight() const
{
	return m_light;
}



bool Entity::HasParticleEmitter() const
{
	bool exists = false;

	if (m_particleEmitter != nullptr)
	{
		exists = true;
	}

	return exists;
}



void Entity::AddParticleEmitter()
{
	if (m_particleEmitter == nullptr)
	{
		m_particleEmitter = new ParticleEmitter();
		GetOwner()->GetRenderScene()->AddParticleEmitter(m_particleEmitter);
	}
}



ParticleEmitter* Entity::GetParticleEmitter()
{
	return m_particleEmitter;
}



const ParticleEmitter* const Entity::GetParticleEmitter() const
{
	return m_particleEmitter;
}



bool Entity::HasHealthPool() const
{
	bool exists = false;

	if (m_healthPool != nullptr)
	{
		exists = true;
	}

	return exists;
}



void Entity::AddHealthPool(float max, float current)
{
	if (m_healthPool == nullptr)
	{
		m_healthPool = new HealthPool(this, max, current);
	}
}



HealthPool* Entity::GetHealthPool()
{
	return m_healthPool;
}



const HealthPool* const Entity::GetHealthPool() const
{
	return m_healthPool;
}



void Entity::Update(float deltaSeconds)
{
	UpdateSelf(deltaSeconds);
	UpdateComponents(deltaSeconds);

	for (int i = 0; i < (int)m_children.size(); ++i)
	{
		m_children[i]->Update(deltaSeconds);
	}

	m_age += deltaSeconds;
}



void Entity::UpdateComponents(float deltaSeconds)
{
	if (m_renderable != nullptr)
	{
		m_renderable->SetModelMatrix(m_transform.GetLocalToWorldMatrix());
	}

	if (m_light != nullptr)
	{
		Matrix4 localToWorld = m_transform.GetLocalToWorldMatrix();
		m_light->SetPosition(localToWorld.GetTranslation());
		m_light->SetDirection(localToWorld.GetForward());
	}

	if (m_particleEmitter != nullptr)
	{
		m_particleEmitter->SetTransform(m_transform.GetLocalToWorldMatrix());
		m_particleEmitter->Update(deltaSeconds);
	}
}



float Entity::GetCollisionRadius() const
{
	return m_collisionRadius;
}



void Entity::SetCollisionRadius(float radius)
{
	m_collisionRadius = radius;
}



bool Entity::CanBeDestroyed()
{
	bool canBeDestroyed = false;

	if (m_wantsToDie == true)
	{
		if (GetParticleEmitter() == nullptr || GetParticleEmitter()->CanBeDestroyed())
		{
			canBeDestroyed = true;
		}
	}

	return canBeDestroyed;
}



void Entity::QueueForDestruction()
{
	m_wantsToDie = true;
}



float Entity::GetAgeInSeconds() const
{
	return m_age;
}