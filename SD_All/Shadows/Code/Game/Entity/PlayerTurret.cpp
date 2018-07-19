#include "Game/Entity/PlayerTurret.hpp"

#include "Engine/Rendering/DebugRender.hpp"
#include "Engine/Profiler/ProfilerVisualizer.hpp"

#include "Engine/Rendering/Renderable.hpp"
#include "Engine/Rendering/Mesh.hpp"
#include "Engine/Rendering/MeshUtils.hpp"
#include "Engine/Core/Transform.hpp"

#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"

#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Stopwatch.hpp"

#include "Game/GameState/PlayState.hpp"
#include "Engine/Rendering/Camera.hpp"
#include "Engine/Math/Ray3.hpp"
#include "Game/Raycast/RaycastHit3.hpp"
#include "Game/Terrain/Terrain.hpp"

extern InputSystem*			g_theInput;
extern AudioSystem*			g_theAudio;
extern Clock*				g_theGameClock;
extern ProfilerVisualizer*	g_theProfiler;	



Entity* CreateBarrelEntity(PlayState* owner, Mesh* mesh)
{
	Entity* barrel = new Entity(owner);

	barrel->AddRenderable();
	Renderable* renderable = barrel->GetRenderable();
	mesh = CreateCube(Vector3(0.0f, 0.0f, 0.5f), Vector3(0.0625f, 0.0625f, .5f));
	renderable->SetMesh(mesh);
	renderable->SetMaterial("Ship");

	return barrel;
}



Entity* CreateLazerEntity(PlayState* owner)
{
	Entity* lazer = new Entity(owner);

	lazer->AddRenderable();
	Renderable* renderable = lazer->GetRenderable();
	Mesh* mesh = CreateLineSegment(Vector3::ZEROS, RGBA(255,0,0), Vector3::ZEROS, RGBA(255,0,0));
	renderable->SetMesh(mesh);
	renderable->SetMaterial("DebugDraw_3D_depth");
	renderable->SetShadowCastingState(false);

	return lazer;
}



PlayerTurret::PlayerTurret(PlayState* owner)
	: Entity(owner)
	, m_fireStopwatch(g_theGameClock)
{
	m_fireStopwatch.SetTimer(1.0f / m_bulletsPerSecond);

	m_busterClock = new Clock();
	m_busterClock->Initialize(g_theGameClock);
	m_busterClock->Pause();
	m_busterRechargeStopwatch = new Stopwatch(m_busterClock);
	m_busterRechargeStopwatch->SetTimer(10.0f);
	m_busterChargeStopwatch = new Stopwatch(g_theGameClock);
	m_busterChargeStopwatch->SetTimer(3.0f);

	AddRenderable();
	Renderable* renderable = GetRenderable();
	m_mesh = CreateUVSphere(Vector3::ZEROS, 0.25f, 10, 10, RGBA());
	renderable->SetMesh(m_mesh);
	renderable->SetMaterial("Ship");

	Entity* barrel = CreateBarrelEntity(owner, m_barrelMesh);
	AddChild(barrel);

	m_lazer = CreateLazerEntity(owner);
	AddChild(m_lazer);

	GetTransform().SetLocalPosition(Vector3(0.0f, 0.25f, 0.0f));
}



PlayerTurret::~PlayerTurret()
{

}



void PlayerTurret::UpdateSelf(float deltaSeconds)
{
	//DebugDraw_Basis(0.0f, GetTransform().GetLocalToWorldMatrix(), RGBA(), RGBA(), DEBUG_RENDER_IGNORE_DEPTH);

	if (!g_theProfiler->IsCapturingMouse())
	{
		Orient(deltaSeconds);
		TryFire(deltaSeconds);
	}
	UpdateLazer();
}


#include "Game/Raycast/RaycastUtils.hpp"
#include "Engine/Math/Sphere.hpp"
#include "Engine/Math/AABB3.hpp"
void PlayerTurret::Orient(float deltaSeconds)
{
	// Shoot a ray from the camera
	Camera* camera = GetOwner()->GetSceneCamera();
	Ray3 ray = Ray3(camera->GetPosition(), camera->GetForward());
	RaycastHit3 hitResult = GetOwner()->Raycast(ray, 64.0f * 2.0f);
	
	// Check to see if we hit anything
	Vector3 lookPosition;
	if (hitResult.hit)
	{
		lookPosition = hitResult.position;
	}
	else
	{
		// If we didn't fake a position really far away from the camera
		lookPosition = ray.GetPositionAtDistance(64.0f * 2.0f);
	}
	DebugDraw_WireSphere(0.0f, lookPosition, 0.1f, RGBA(255,0,0), RGBA(255,0,0), DEBUG_RENDER_USE_DEPTH);


	// Draw a line from our nose to the look position
	Vector3 noseWorld = (GetTransform().GetLocalToWorldMatrix() * Vector4(m_nose, 1.0f)).XYZ();
	//DebugDraw_LineSegment(0.0f, noseWorld, RGBA(255,0,0), lookPosition, RGBA(255,0,0), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);


	// Transform into parent space
	Vector3 lookPosition_Parent = (GetTransform().GetWorldToParentMatrix() * Vector4(lookPosition, 1.0f)).XYZ();
	Vector3 lookDirection_Parent = lookPosition_Parent - GetTransform().GetLocalPosition();
	Quaternion lookAtQuat_Parent = Quaternion::LookAt(lookDirection_Parent);
	

	// Rotate toward the new quaternion
	Quaternion newOrientation = QuaternionRotateTorward(GetTransform().GetLocalOrientation(), lookAtQuat_Parent, m_turnRateDegrees * deltaSeconds);
	GetTransform().SetLocalOrientation(newOrientation);
}



void PlayerTurret::TryFire(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	Matrix4 localToWorld = GetTransform().GetLocalToWorldMatrix();
	Vector3 noseWorld = (localToWorld * Vector4(m_nose, 1.0f)).XYZ();
	DebugDraw_WireSphere(0.0f, noseWorld, 0.1f, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);


	// Fire the chain gun
	if (g_theInput->WasMouseButtonJustPressed(InputSystem::MOUSE_LEFT) || g_theInput->WasKeyJustPressed(' '))
	{
		m_fireStopwatch.Reset();
	}
	if (g_theInput->IsMouseButtonPressed(InputSystem::MOUSE_LEFT))
	{
		int bulletsToSpawn = (int)m_fireStopwatch.DecrementAll();
		for (int i = 0; i < bulletsToSpawn; ++i)
		{
			Vector3 offset = Vector3(GetRandomFloatInRange(-m_bulletSpreadHalfAngle, m_bulletSpreadHalfAngle), GetRandomFloatInRange(-m_bulletSpreadHalfAngle, m_bulletSpreadHalfAngle), 0.0f);
			
			GetOwner()->SpawnBullet(noseWorld, GetTransform().GetForward(), 50.0f, 25.0f, "!BlueTeam", 0.25f, 2.5f, 0.08f);
			g_theAudio->PlayRandomSoundFromGroup("TankFire");
		}
	}


	// Fire Buster
	// Our buster shot has recharged
	//if (m_busterRechargeStopwatch->DecrementAll() > 0)
	//{
	//	m_busterRechargeStopwatch->Reset();
	//	m_busterClock->Pause();
	//	m_canFireBuster = true;
	//}


	// Charging the buster shot
	//if (m_canFireBuster)
	//{
	//	if (g_theInput->WasMouseButtonJustPressed(InputSystem::MOUSE_RIGHT))
	//	{
	//		m_busterChargeStopwatch->Reset();
	//	}
	//	if (g_theInput->IsMouseButtonPressed(InputSystem::MOUSE_RIGHT))
	//	{
	//		if (m_busterChargeStopwatch->DecrementAll() > 0)
	//		{
	//			// Fire!
	//			m_busterClock->Unpause();
	//			m_busterRechargeStopwatch->Reset();
	//			m_canFireBuster = false;
	//
	//			DebugDraw_WireSphere(5.0f, noseWorld, 1.0f, RGBA(255,0,0), RGBA(255,0,0), DEBUG_RENDER_USE_DEPTH);
	//		}
	//	}
	//}
}



void PlayerTurret::UpdateLazer()
{
 	Matrix4 localToWorld = GetTransform().GetLocalToWorldMatrix();
	Matrix4 worldToLocal = GetTransform().GetWorldToLocalMatrix();
	Vector3 noseWorld = (localToWorld * Vector4(m_nose, 1.0f)).XYZ();

	// Destroy the old mesh
	delete m_lazer->GetRenderable()->GetMesh();

	// Get end point for new mesh
	// Shoot a ray from the camera
	Ray3 ray = Ray3(noseWorld, GetTransform().GetForward());
	RaycastHit3 hitResult = GetOwner()->Raycast(ray, 64.0f * 2.0f);

	// Check to see if we hit anything
	Vector3 lookPosition;
	if (hitResult.hit)
	{
		lookPosition = hitResult.position;
	}
	else
	{
		// If we didn't fake a position really far away from the camera
		lookPosition = ray.GetPositionAtDistance(64.0f * 2.0f);
	}
	DebugDraw_WireSphere(0.0f, lookPosition, 0.125f, RGBA(0,0,255), RGBA(0,0,255), DEBUG_RENDER_USE_DEPTH);
	lookPosition = (worldToLocal * Vector4(lookPosition, 1.0f)).XYZ();


	Mesh* mesh = CreateLineSegment(m_nose, m_lazerColor, lookPosition, m_lazerColor);
	//Mesh* mesh = CreateLineSegment(Vector3::ZEROS, m_lazerColor, Vector3(0.0f, 0.0f, 64.0f), m_lazerColor);
	m_lazer->GetRenderable()->SetMesh(mesh);
}
