#pragma once

#include "Game/Entity/Entity.hpp"

class Mesh;



class BombEnemy : public Entity
{
public:
	BombEnemy(PlayState* owner);
	virtual ~BombEnemy() override;

	virtual void UpdateSelf(float deltaSeconds) override;
	void Move(float deltaSeconds);
	Vector3 GetDirectionToPlayer() const;
	Vector3 SeperateFromFriends() const;
	Vector3 GetDirectionToCenterOfFriends() const;
	Vector3 GetAverageDirectionOfFriends() const;
	void TryExplode();

	virtual void OnCollision(Entity* other) override;


private:
	// Renderable
	Mesh* m_mesh = nullptr;

	// Movement
	float m_movementSpeed = 3.0f;
	float m_turnRateDegrees = 135.0f;
	
	// Explosion
	float m_explosionRadius = 0.5f;
	float m_explosionDamage = 100.0f;

	// FLOCK
	static constexpr float	SEPERATION_DISTANCE = 2.0f; 

	static constexpr float	SEEK_WEIGHT			= 3.0f;
	static constexpr float	SEPERATION_WEIGHT	= 3.0f;
	static constexpr float	COHESION_WEIGHT		= 1.0f;
	static constexpr float	ALIGNMENT_WEIGHT	= 1.0f;
};

