#include "Game/Entity/PlayerTank.hpp"

#include "Engine/Rendering/DebugRender.hpp"

#include "Engine/Rendering/Mesh.hpp"
#include "Engine/Rendering/MeshUtils.hpp"
#include "Engine/Rendering/Renderable.hpp"

#include "Game/Terrain/Terrain.hpp"
#include "Game/GameState/PlayState.hpp"
#include "Game/Entity/PlayerTurret.hpp"

#include "Game/Components/HealthPool.hpp"

#include "Engine/Core/Clock.hpp"
#include "Engine/Input/InputSystem.hpp"
extern Clock*		g_theGameClock;
extern InputSystem* g_theInput;



PlayerTank::PlayerTank(PlayState* owner)
	: Entity(owner)
	, m_breadcrumbStopwatch(g_theGameClock)
{
	// Renderable
	AddRenderable();
	Renderable* renderable = GetRenderable();
	m_mesh = CreateCube(Vector3::ZEROS, Vector3(0.375f, 0.25f, 0.75f));
	renderable->SetMesh(m_mesh);
	renderable->SetMaterial("Ship");
	//renderable->SetShadowReceivingState(false);


	// Health
	AddHealthPool(500.0f);

	// Tags
	GetTags().Set("BlueTeam");


	// Breadcrumbs
	m_breadcrumbStopwatch.SetTimer(BREADCRUMB_INTERVAL);


	// Turret
	Entity* turret = new PlayerTurret(GetOwner());
	AddChild(turret);
}



PlayerTank::~PlayerTank()
{
}



void PlayerTank::UpdateSelf(float deltaSeconds)
{
	Move(deltaSeconds);
	TrySpawnBreadcrumb();
}



void PlayerTank::TrySpawnBreadcrumb()
{
	//DebugDraw_Basis(0.0f, GetTransform().GetLocalToWorldMatrix(), RGBA(), RGBA(), DEBUG_RENDER_IGNORE_DEPTH);
	
	unsigned int numBreadcrumbs = m_breadcrumbStopwatch.DecrementAll(); 
	for (int i = 0; i < (int)numBreadcrumbs; ++i)
	{
		//DebugDraw_Point(3.0f, GetTransform().GetWorldPosition(), RGBA(0, 255, 0, 255), RGBA(255, 0, 0, 0), DEBUG_RENDER_USE_DEPTH);
	}
}



void PlayerTank::Move(float deltaSeconds)
{
	const Terrain* terrain = GetOwner()->GetTerrain();
	Transform& transform = GetTransform();


	// Rotation
	Vector3 terrainNormal = Vector3::UP;
	if (terrain->IsOnTerrain(transform.GetWorldPosition()))
	{

		terrainNormal = terrain->GetNormal(GetTransform().GetWorldPosition());
	}
	if (g_theInput->IsKeyPressed('Q') || g_theInput->IsKeyPressed('A'))
	{
		transform.RotateAroundWorldAxis(Vector3::UP, -m_rotationSpeed * deltaSeconds);
	}
	if (g_theInput->IsKeyPressed('E') || g_theInput->IsKeyPressed('D'))
	{
		transform.RotateAroundWorldAxis(Vector3::UP, m_rotationSpeed * deltaSeconds);
	}


	// Movement
	// Get the direction we want to move
	Vector3 movementDirection = Vector3::ZEROS;
	if (g_theInput->IsKeyPressed('W'))
	{
		movementDirection += Vector3::FORWARD;
	}
	if (g_theInput->IsKeyPressed('S'))
	{
		movementDirection += -Vector3::FORWARD;
	}

	// If we want to move
	if (movementDirection.GetLengthSquared() > 0.0f)
	{
		// Actually do the move
		Vector3 worldMovementDirection = (transform.GetLocalToWorldMatrix() * Vector4(movementDirection, 0.0f)).XYZ();
		worldMovementDirection.y = 0.0f;
		worldMovementDirection.NormalizeAndGetLength();
		transform.TranslateWorld(worldMovementDirection * deltaSeconds* m_movementSpeed);
	}

	// If we are on the terrain
	Vector3 worldPosition = transform.GetWorldPosition();
	if (terrain->IsOnTerrain(worldPosition))
	{
		// Pop us up to the top of the terrain
		float newHeight = terrain->GetHeight(worldPosition);
		Vector3 newWorldPosition = Vector3(worldPosition.x, newHeight + 0.4f, worldPosition.z);
		transform.SetWorldPosition(newWorldPosition);


		// Correct orientation to terrain
		Vector3 terrainNormal_World = terrain->GetNormal(transform.GetWorldPosition());
		Vector3 newRight_World		= CrossProduct(terrainNormal, transform.GetForward());
		Vector3 newForward_World	= CrossProduct(newRight_World, terrainNormal_World);

		Matrix4 worldToParentMatrix  = transform.GetWorldToParentMatrix();
		Vector3 terrainNormal_Parent = (worldToParentMatrix * Vector4(terrainNormal_World, 0.0f)).XYZ();
		Vector3 newRight_Parent		 = (worldToParentMatrix * Vector4(newRight_World, 0.0f)).XYZ();
		Vector3 newForward_Parent	 = (worldToParentMatrix * Vector4(newForward_World, 0.0f)).XYZ();

		Matrix4 newRotationMatrix = Matrix4(newRight_Parent, terrainNormal_Parent, newForward_Parent, Vector3(0.0f, 0.0f, 0.0f));
		Quaternion orientation = Quaternion::FromMatrix(newRotationMatrix);
		transform.SetLocalOrientation(orientation);
	}
}