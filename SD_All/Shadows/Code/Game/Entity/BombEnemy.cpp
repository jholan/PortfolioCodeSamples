#include "Game/Entity/BombEnemy.hpp"

#include "Engine/Rendering/MeshUtils.hpp"
#include "Engine/Rendering/Renderable.hpp"
#include "Engine/Rendering/DebugRender.hpp"
#include "Engine/Rendering/Material.hpp"
#include "Engine/Rendering/Camera.hpp"

#include "Game/Terrain/Terrain.hpp"

#include "Game/GameState/PlayState.hpp"
#include "Engine/Rendering/ForwardRenderPath.hpp"
#include "Game/Entity/Entity.hpp"
#include "Game/Components/HealthPool.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Game/GameCommon.hpp"


Entity* CreateEye(Entity* owner, bool isLeft)
{
	Entity* eye = new Entity(owner->GetOwner());

	eye->AddRenderable();
	Renderable* renderable = eye->GetRenderable();
	Mesh* mesh = CreateUVSphere(Vector3::ZEROS, 0.1f, 2, 2, RGBA());
	renderable->SetMesh(mesh);
	renderable->SetMaterial("Eye");

	Vector3 position = Vector3::ZEROS;
	position.y = owner->GetCollisionRadius() * 0.4f;
	position.z = owner->GetCollisionRadius() * 0.4f;
	if (isLeft)
	{
		position.x -= 0.25f;
	}
	else
	{
		position.x += 0.25f;
	}


	eye->GetTransform().SetLocalPosition(position);

	return eye;
}



BombEnemy::BombEnemy(PlayState* owner)
	: Entity(owner)
{
	// Renderable
	AddRenderable();
	m_mesh = CreateUVSphere(Vector3::ZEROS, 0.5f, 10, 10, RGBA());
	Renderable* renderable = GetRenderable();
	renderable->SetMesh(m_mesh);
	renderable->SetMaterial("Bomber");

	Entity* leftEye	 = CreateEye(this, true);
	Entity* rightEye = CreateEye(this, false);
	AddChild(leftEye);
	AddChild(rightEye);

	// HealthPool
	AddHealthPool();

	// Tags
	GetTags().Set("RedTeam");


	// Collision
	SetCollisionRadius(m_explosionRadius);
}



BombEnemy::~BombEnemy()
{

}



void BombEnemy::UpdateSelf(float deltaSeconds)
{
	//DebugDraw_WireSphere(0.0f, GetTransform().GetWorldPosition(), GetCollisionRadius(), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);

	// Kill ourselves if we have no health
	if (GetHealthPool()->GetHealth() <= 0.0f)
	{
		SoundID deathSID = g_theAudio->CreateOrGetSound("Data/Audio/EnemyDied.wav");
		g_theAudio->PlaySound(deathSID, false, 0.33f);
		QueueForDestruction();
	}

	Move(deltaSeconds);
	TryExplode();


	// UI
	HealthPool* healthPool = GetHealthPool();
	float currentHealth = healthPool->GetHealth();
	float maxHealth = healthPool->GetMaxHealth();
	float normalizedHealth = currentHealth / maxHealth;
	if (currentHealth < maxHealth)
	{
		Vector3 offset = Vector3(0.0f, 1.0f, 0.0f);

		Vector3 right = GetOwner()->GetSceneCamera()->GetRight();
		Vector3 up = GetOwner()->GetSceneCamera()->GetUp();
		Vector3 forward = GetOwner()->GetSceneCamera()->GetForward();

		Renderable* background = new Renderable();
		Mesh* bgMesh = CreatePlane(GetTransform().GetWorldPosition() + offset, Vector2(0.5f, 0.1f), right, up);
		background->SetMesh(bgMesh);
		background->SetMaterial("DebugDraw_3D_depth");
		background->GetMaterial()->SetProperty("TINT", RGBA(0,0,0));
		GetOwner()->GetForwardRenderPass()->AddTemporaryRenderable(background);

		Renderable* foreground = new Renderable();
		Mesh* fgMesh = CreatePlane(GetTransform().GetWorldPosition() + offset + (forward * -0.01f), Vector2(0.475f * normalizedHealth, 0.075f), right, up);
		foreground->SetMesh(fgMesh);
		foreground->SetMaterial("DebugDraw_3D_depth");
		foreground->GetMaterial()->SetProperty("TINT", RGBA(255,0,0));
		GetOwner()->GetForwardRenderPass()->AddTemporaryRenderable(foreground);
	}

	//DebugDraw_TextTag(0.0f, GetTransform().GetWorldPosition(), 12.0f, RGBA(), RGBA(), "BE.hp %f", GetHealthPool()->GetHealth());
	//DebugDraw_Basis(0.0f, GetTransform().GetLocalToWorldMatrix(), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
}



Vector3 BombEnemy::GetDirectionToPlayer() const
{
	const Transform& transform = GetTransform();
	Matrix4 worldToParent = transform.GetWorldToParentMatrix();


	// Turn toward the player
	Entity* player = (Entity*)GetOwner()->GetPlayer();
	Vector3 playerPosition_World = player->GetTransform().GetWorldPosition();
	Vector3 playerPosition_Parent = (worldToParent * Vector4(playerPosition_World, 1.0f)).XYZ();
	Vector3 directionToPlayer_Parent = (playerPosition_Parent - transform.GetLocalPosition()).GetNormalized();


	directionToPlayer_Parent.y = 0.0f;
	directionToPlayer_Parent.NormalizeAndGetLength();
	return directionToPlayer_Parent;
}



Vector3 BombEnemy::SeperateFromFriends() const
{
	GUARANTEE_OR_DIE(SEPERATION_DISTANCE != 0.0f, "SEPERATION_DISTANCE cannot be 0.0f");

	Vector3 direction = Vector3::ZEROS;


	// Cached Values
	const Transform& transform = GetTransform();
	Matrix4 worldToParent = transform.GetWorldToParentMatrix();
	const std::vector<Entity*>& bombEnemies = GetOwner()->GetBombEnemies();


	// Find the closest bomber
	Entity* closestNeighbor = nullptr;
	float	distanceToClosestNeighbor = 10000000000.0f;
	for (int i = 0; i < (int)bombEnemies.size(); ++i)
	{
		Entity* bombEnemy = bombEnemies[i];
		
		// If we are comparing against ourself skip
		if (bombEnemy == this)
		{
			continue;
		}

		float distanceToBombEnemy = (bombEnemy->GetTransform().GetWorldPosition() - transform.GetWorldPosition()).GetLength();
		if (distanceToBombEnemy < distanceToClosestNeighbor)
		{
			distanceToClosestNeighbor = distanceToBombEnemy;
			closestNeighbor = bombEnemy;
		}
	}


	// If we are closer to a neighbor than we would like, get the direction we want to move to get away from them
	if (distanceToClosestNeighbor < SEPERATION_DISTANCE)
	{
		Vector3 cnPosition_World = closestNeighbor->GetTransform().GetWorldPosition();
		Vector3 cnPosition_Parent = (worldToParent * Vector4(cnPosition_World, 1.0f)).XYZ();
		Vector3 directionToCN_Parent = (cnPosition_Parent - transform.GetLocalPosition()).GetNormalized();
		//DebugDraw_LineSegment(0.0f, transform.GetWorldPosition(), RGBA(), cnPosition_World, RGBA(), RGBA(), RGBA(), DEBUG_RENDER_IGNORE_DEPTH);

		Vector3 seperationDirection_Parent = -directionToCN_Parent;

		direction = seperationDirection_Parent;
	}


	// Calculate how compressed we are
	float compressionRatio = 1.0f - (distanceToClosestNeighbor / SEPERATION_DISTANCE);



	direction.y = 0.0f;
	direction.NormalizeAndGetLength();
	direction *= compressionRatio;
	return direction;
}



Vector3 BombEnemy::GetDirectionToCenterOfFriends() const
{
	// Cached Values
	const Transform& transform = GetTransform();
	Matrix4 worldToParent = transform.GetWorldToParentMatrix();
	const std::vector<Entity*>& bombEnemies = GetOwner()->GetBombEnemies();


	// Determine the center of all the bomb enemies
	Vector3 unweightedCenter_World = Vector3::ZEROS;
	for (int i = 0; i < (int)bombEnemies.size(); ++i)
	{
		unweightedCenter_World += bombEnemies[i]->GetTransform().GetWorldPosition();
	}

	Vector3 weightedCenter_World = unweightedCenter_World / (float)(bombEnemies.size());
	Vector3 weightedCenter_Parent = (worldToParent * Vector4(weightedCenter_World, 1.0f)).XYZ();
	Vector3 directionToCenter_Parent = (weightedCenter_Parent - transform.GetLocalPosition()).GetNormalized();

	//DebugDraw_WireSphere(0.0f, weightedCenter_World, 0.25f, RGBA(), RGBA(), DEBUG_RENDER_IGNORE_DEPTH);


	directionToCenter_Parent.y = 0.0f;
	directionToCenter_Parent.NormalizeAndGetLength();
	return directionToCenter_Parent;
}



Vector3 BombEnemy::GetAverageDirectionOfFriends() const
{
	// Cached Values
	const Transform& transform = GetTransform();
	Matrix4 worldToParent = transform.GetWorldToParentMatrix();
	const std::vector<Entity*>& bombEnemies = GetOwner()->GetBombEnemies();


	// Determine the center of all the bomb enemies
	Vector3 unweightedForward_World = Vector3::ZEROS;
	for (int i = 0; i < (int)bombEnemies.size(); ++i)
	{
		unweightedForward_World += bombEnemies[i]->GetTransform().GetForward();
	}

	Vector3 weightedForward_World = unweightedForward_World / (float)(bombEnemies.size());
	Vector3 weightedForward_Parent = (worldToParent * Vector4(weightedForward_World, 1.0f)).XYZ();

	//DebugDraw_LineSegment(0.0f, transform.GetWorldPosition() + Vector3(0.0f, 2.0f, 0.0f), RGBA(), transform.GetWorldPosition() + Vector3(0.0f, 2.0f, 0.0f) + weightedForward_World, RGBA(), RGBA(), RGBA(), DEBUG_RENDER_IGNORE_DEPTH);

	weightedForward_Parent.y = 0.0f;
	weightedForward_Parent.NormalizeAndGetLength();
	return weightedForward_Parent;
}



void BombEnemy::Move(float deltaSeconds)
{
	Transform& transform = GetTransform();
	Matrix4 worldToParent = transform.GetWorldToParentMatrix();

	// Get the direction for all of the behaviors
	Vector3 directionToPlayer_Parent = GetDirectionToPlayer();
	Vector3 seperationDirection_Parent = SeperateFromFriends();
	Vector3 flockCenterDirection_Parent = GetDirectionToCenterOfFriends();
	Vector3 flockAverageDirection_Parent = GetAverageDirectionOfFriends();
	
	// Weight the direction
	Vector3 weightedMoveDirection = (SEEK_WEIGHT * directionToPlayer_Parent) + (SEPERATION_WEIGHT * seperationDirection_Parent) + (COHESION_WEIGHT * flockCenterDirection_Parent) + (ALIGNMENT_WEIGHT * flockAverageDirection_Parent);
	weightedMoveDirection.NormalizeAndGetLength();
	
	Quaternion lookAtQuat_Parent = Quaternion::LookAt(weightedMoveDirection);

	Quaternion newOrientation = QuaternionRotateTorward(transform.GetLocalOrientation(), lookAtQuat_Parent, m_turnRateDegrees * deltaSeconds);
	transform.SetLocalOrientation(newOrientation);


	// Move forward
	Vector3 forward_World = transform.GetForward();
	transform.TranslateWorld(forward_World * deltaSeconds * m_movementSpeed);


	// Snap ourselves to the top of the terrain
	if (GetOwner()->GetTerrain()->IsOnTerrain(transform.GetWorldPosition()))
	{
		Vector3 terrainPosition = GetOwner()->GetTerrain()->GetTerrainPosition(transform.GetWorldPosition());
		transform.SetWorldPosition(terrainPosition + Vector3(0.0f, 0.5f, 0.0f)); // Add an offset so we aren't halfway inside the terrain
	}
}



void BombEnemy::TryExplode()
{
	Entity* player = (Entity*)GetOwner()->GetPlayer();
	Vector3 playerPosition = player->GetTransform().GetWorldPosition();
	Vector3 directionToPlayerWorld = playerPosition - GetTransform().GetWorldPosition();
	float distanceToPlayer = directionToPlayerWorld.NormalizeAndGetLength();

	if (distanceToPlayer <= m_explosionRadius)
	{
		//DebugDraw_Log(0.0f, "Exploding");
	}
}



void BombEnemy::OnCollision(Entity* other)
{
	if (other->HasHealthPool())
	{
		bool didDamage = other->GetHealthPool()->DecrementHealth(m_explosionDamage, "!RedTeam");
		if (didDamage)
		{
			SoundID deathSID = g_theAudio->CreateOrGetSound("Data/Audio/EnemyDied.wav");
			g_theAudio->PlaySound(deathSID, false, 0.5f);
			QueueForDestruction();
		}
	}
}
