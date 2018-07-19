#include "Game/Entity/BombEnemySpawner.hpp"

#include "Engine/Rendering/DebugRender.hpp"

#include "Engine/Core/Stopwatch.hpp"

#include "Engine/Rendering/Renderable.hpp"
#include "Engine/Rendering/Mesh.hpp"
#include "Engine/Rendering/MeshUtils.hpp"
#include "Engine/Rendering/Material.hpp"
#include "Engine/Rendering/Camera.hpp"
#include "Engine/Rendering/ForwardRenderPath.hpp"

#include "Game/GameState/PlayState.hpp"
#include "Game/Entity/BombEnemy.hpp"
#include "Game/Terrain/Terrain.hpp"

#include "Game/Components/HealthPool.hpp"

#include "Game/GameCommon.hpp"
#include "Engine/Core/Clock.hpp"
extern Clock* g_theGameClock;



BombEnemySpawner::BombEnemySpawner(PlayState* owner, unsigned int maxChildren, float spawnTime)
	: Entity(owner)
{
	m_maxChildren = maxChildren;
	m_spawnTime = spawnTime;

	m_spawnStopwatch = new Stopwatch(g_theGameClock);
	m_spawnStopwatch->SetTimer(m_spawnTime);

	AddHealthPool(1000.0f);

	SetCollisionRadius(2.0f);

	GetTags().Set("RedTeam");

	AddRenderable();
	Renderable* renderable = GetRenderable();
	Mesh* mesh = CreateCube(Vector3(0.0f, 0.0f, 0.0f), Vector3(GetCollisionRadius(), GetCollisionRadius(), GetCollisionRadius()), RGBA());
	renderable->SetMesh(mesh);
	renderable->SetMaterial("Spawner");
}



BombEnemySpawner::~BombEnemySpawner()
{

}



void BombEnemySpawner::UpdateSelf(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	if (GetHealthPool()->GetHealth() <= 0.0f)
	{
		SoundID deathSID = g_theAudio->CreateOrGetSound("Data/Audio/EnemyDied.wav");
		g_theAudio->PlaySound(deathSID, false, 0.66f);
		QueueForDestruction();
	}

	Vector3 newPositionWorld = GetTransform().GetWorldPosition();
	float newHeight = GetOwner()->GetTerrain()->GetHeight(newPositionWorld);
	newPositionWorld.y = newHeight + GetCollisionRadius();
	GetTransform().SetWorldPosition(newPositionWorld);
	//DebugDraw_WireSphere(0.0f, GetTransform().GetWorldPosition(), m_spawnDistance, RGBA(255,0,0), RGBA(255,0,0), DEBUG_RENDER_USE_DEPTH);

	RemoveDeadChildren();
	TrySpawn();


	// UI
	HealthPool* healthPool = GetHealthPool();
	float currentHealth = healthPool->GetHealth();
	float maxHealth = healthPool->GetMaxHealth();
	float normalizedHealth = currentHealth / maxHealth;
	if (currentHealth < maxHealth)
	{
		Vector3 offset = Vector3(0.0f, 1.0f + GetCollisionRadius(), 0.0f);

		Vector3 right = GetOwner()->GetSceneCamera()->GetRight();
		Vector3 up = GetOwner()->GetSceneCamera()->GetUp();
		Vector3 forward = GetOwner()->GetSceneCamera()->GetForward();

		Renderable* background = new Renderable();
		Mesh* bgMesh = CreatePlane(GetTransform().GetWorldPosition() + offset, Vector2(GetCollisionRadius(), 0.2f), right, up);
		background->SetMesh(bgMesh);
		background->SetMaterial("DebugDraw_3D_depth");
		background->GetMaterial()->SetProperty("TINT", RGBA(0,0,0));
		GetOwner()->GetForwardRenderPass()->AddTemporaryRenderable(background);

		Renderable* foreground = new Renderable();
		Mesh* fgMesh = CreatePlane(GetTransform().GetWorldPosition() + offset + (forward * -0.05f), Vector2( GetCollisionRadius() * .975f * normalizedHealth, 0.175f), right, up);
		foreground->SetMesh(fgMesh);
		foreground->SetMaterial("DebugDraw_3D_depth");
		foreground->GetMaterial()->SetProperty("TINT", RGBA(255,0,0));
		GetOwner()->GetForwardRenderPass()->AddTemporaryRenderable(foreground);
	}
	//DebugDraw_TextTag(0.0f, GetTransform().GetWorldPosition(), 16.0f, RGBA(), RGBA(), "BES.hp %f", GetHealthPool()->GetHealth());
}



void BombEnemySpawner::RemoveDeadChildren()
{
	PlayState* owner = GetOwner();
	for (int i = (int)m_children.size() - 1; i >= 0; --i)
	{
		bool exists = owner->HasEntityWithPointer(m_children[i]);

		if (!exists)
		{
			m_children.erase(m_children.begin() + i);
		}
	}
}



void BombEnemySpawner::TrySpawn()
{
	// Short Circuit.
	if ((int)m_children.size() >= m_maxChildren)
	{
		m_spawnStopwatch->DecrementAll();
		return;
	}
	
	int count = (unsigned int)m_spawnStopwatch->DecrementAll();
	count = ClampInt(count, 0, m_maxChildren - (int)m_children.size());
	for (int i = 0; i < count; ++i)
	{
		Spawn();
	}
}



void BombEnemySpawner::Spawn()
{
	Vector2 xzPosition = GetRandomVector2() * m_spawnDistance;
	BombEnemy* be = GetOwner()->SpawnBombEnemy(GetTransform().GetWorldPosition().XZ() + xzPosition);
	m_children.push_back(be);
}