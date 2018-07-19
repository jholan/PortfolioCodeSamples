#pragma once
#include "Game/Entity/Entity.hpp"

#include <string>

class Mesh;



class Bullet : public Entity
{
public:
	Bullet(PlayState* owner, const Vector3& startPosition, const Vector3& direction, float speed, float damage, const std::string& damageConditions, float damageRadius, float lifetimeSeconds, float physicalRadius);
	virtual ~Bullet() override;

	virtual void UpdateSelf(float deltaSeconds) override;

	virtual void OnCollision(Entity* other) override;


private:
	// Damage
	float		m_damage;
	std::string m_damageConditions;
	float		m_damageRadius;

	// Movement
	float		m_moveSpeed;
	Vector3		m_moveDirection;

	// Lifetime
	float		m_maxLifetimeSeconds;

	// Rendering
	Mesh*		m_mesh = nullptr;
};