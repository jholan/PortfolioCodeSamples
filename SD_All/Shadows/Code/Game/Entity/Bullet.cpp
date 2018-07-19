#include "Game/Entity/Bullet.h"

#include "Engine/Rendering/DebugRender.hpp"

#include "Engine/Rendering/Mesh.hpp"
#include "Engine/Rendering/MeshUtils.hpp"
#include "Engine/Rendering/Renderable.hpp"
#include "Engine/Rendering/Light.hpp"

#include "Game/Components/HealthPool.hpp"

#include "Game/GameState/PlayState.hpp"
#include "Game/Terrain/Terrain.hpp"



Bullet::Bullet(PlayState* owner, const Vector3& startPosition, const Vector3& direction, float speed, float damage, const std::string& damageConditions, float damageRadius, float lifetimeSeconds, float physicalRadius)
	: Entity(owner)
{
	// Renderable
	AddRenderable();
	Renderable* renderable = GetRenderable();
	m_mesh = CreateCube(Vector3::ZEROS, Vector3(physicalRadius, physicalRadius, physicalRadius));
	renderable->SetMesh(m_mesh);
	renderable->SetMaterial("Bullet");

	// Light
	AddLight();
	Light* light = GetLight();
	light->InitializePointLight(GetTransform().GetWorldPosition(), RGBA(), 0.5f);
	light->SetShadowCastingState(false);

	// Damage
	m_damage = damage;
	m_damageConditions = damageConditions;
	m_damageRadius = damageRadius;

	// Movement
	GetTransform().SetWorldPosition(startPosition);
	m_moveSpeed = speed;
	m_moveDirection = direction;

	// Orientation
	Vector3 right_world = CrossProduct(Vector3::UP, direction);
	Vector3 up_world = CrossProduct(direction, right_world);
	Quaternion orientation = Quaternion::FromMatrix(Matrix4(right_world, up_world, direction));
	GetTransform().SetLocalOrientation(orientation);

	// Lifetime
	m_maxLifetimeSeconds = lifetimeSeconds;

	// Collision
	SetCollisionRadius(damageRadius);
}



Bullet::~Bullet()
{

}



void Bullet::UpdateSelf(float deltaSeconds)
{
	// Lifetime
	if (GetAgeInSeconds() >= m_maxLifetimeSeconds)
	{
		QueueForDestruction();
	}


	// Movement
	Transform& transform = GetTransform();
	transform.TranslateWorld(m_moveDirection * m_moveSpeed * deltaSeconds);


	// Terrain Collisions
	if (GetOwner()->GetTerrain()->IsOnTerrain(transform.GetWorldPosition()))
	{
		float terrainHeight = GetOwner()->GetTerrain()->GetHeight(transform.GetWorldPosition());
		if (terrainHeight >= transform.GetWorldPosition().y)
		{
			QueueForDestruction();
		}
	}

	//DebugDraw_WireAABB3(0.0f, transform.GetWorldPosition(), Vector3(0.08f, 0.08f, 0.08f), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
	//DebugDraw_WireSphere(0.0f, transform.GetWorldPosition(), GetCollisionRadius(), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
}



void Bullet::OnCollision(Entity* other)
{
	// If it can be damaged
	if (other->HasHealthPool())
	{
		other->GetHealthPool()->DecrementHealth(m_damage, m_damageConditions);
	}

	// Destroy the bullet
	QueueForDestruction();
}