#include "Game/GameState/PlayState.hpp"

#include "Engine/Rendering/DebugRender.hpp"
#include "Engine/Profiler/ProfilerVisualizer.hpp"
#include "Engine/Profiler/ProfileScope.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"

#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/Blackboard.hpp"

#include "Engine/Rendering/OrbitCamera.hpp"
#include "Engine/Rendering/Camera.hpp"
#include "Engine/Rendering/Texture.hpp"
#include "Engine/Rendering/TextureCube.hpp"
#include "Engine/Rendering/Material.hpp"
#include "Engine/Rendering/MeshUtils.hpp"
#include "Engine/Rendering/Mesh.hpp"
#include "Engine/Rendering/BitmapFont.hpp"
#include "Engine/Rendering/Light.hpp"
#include "Engine/Rendering/RenderScene.hpp"
#include "Engine/Rendering/ForwardRenderPath.hpp"

#include "Game/Menus/Menu.hpp"

#include "Game/GameCommon.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Rendering/Renderer.hpp"
#include "Engine/Commands/DevConsole.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/App.hpp"

#include "Game/Game.hpp"

#include "Game/Terrain/Terrain.hpp"

#include "Game/Entity/Entity.hpp"
#include "Game/Entity/PlayerTank.hpp"
#include "Game/Entity/BombEnemy.hpp"
#include "Game/Entity/BombEnemySpawner.hpp"
#include "Game/Entity/Bullet.h"

#include "Game/Components/HealthPool.hpp"

#include "Game/Raycast/RaycastUtils.hpp"
#include "Engine/Math/Sphere.hpp"
#include "Engine/Math/AABB3.hpp"


//--TEST-HEADERS-----------------------------------------------------
#include "Engine/Core/StaticScene.hpp"
#include "Engine/Rendering/Renderable.hpp"
#include "Engine/Math/Ray3.hpp"
#include "Game/Raycast/RaycastHit3.hpp"
//-------------------------------------------------------------------

extern Blackboard			g_gameConfigBlackboard;
extern Window*				g_theWindow;
extern Renderer*			g_theRenderer;
extern DevConsole*			g_theDevConsole;
extern Clock*				g_theGameClock;
extern App*					g_theApp;
extern InputSystem*			g_theInput;
extern AudioSystem*			g_theAudio;
extern ProfilerVisualizer*	g_theProfiler;


// DFS Things
static Camera* c1Cam;
static Camera* c2Cam;
static Camera* c3Cam;
void PlayState::Initialize(Game* owner)
{
	GUARANTEE_OR_DIE(owner != nullptr, "CharacterSelectState owner must be valid");


	// Basic State
	SetOwner(owner);
	SetName("PlayState");


	// Pause 
	// Fullscreen Camera
	m_fullScreenCamera = new Camera();
	m_fullScreenCamera->SetProjection(Matrix4::MakeOrtho2D(Vector2(0.0f, 0.0f), (Vector2)g_theWindow->GetDimensions()));
	m_fullScreenCamera->SetClearColor(RGBA(0,0,0));
	m_fullScreenCamera->SetColorTarget(0, g_theRenderer->GetDefaultColorTarget());
	m_fullScreenCamera->SetDepthStencilTarget(g_theRenderer->GetDefaultDepthStencilTarget());
	
	// Menu
	m_pauseMenu = new Menu();
	m_pauseMenu->Initialize(Vector2(0.0f, 0.0f), true, false);
	m_pauseMenu->SetExternalPadding(Vector2(0.0f, 0.0f));
	m_pauseMenu->AddOption("EXIT", PAUSE_MENU_EXIT);
	m_pauseMenu->AddOption("RESUME", PAUSE_MENU_RESUME);
	Vector2 padding = Vector2(g_theWindow->GetDimensions()) - m_pauseMenu->GetDimensions();
	padding.y -= 128.0f;
	m_pauseMenu->SetExternalPadding(padding / 2.0f);

	m_pauseEffect = Material::FromShader("FSFX_Desaturation");


	// Victory
	m_victoryEffect = Material::FromShader("FSFX_Saturation");
	m_victoryEffect->SetProperty("END_COLOR", RGBA(76,175,80));

	m_victoryMenu = new Menu();
	m_victoryMenu->Initialize(Vector2(0.0f, 0.0f), true, false);
	m_victoryMenu->SetExternalPadding(Vector2(0.0f, 0.0f));
	m_victoryMenu->AddOption("Press SPACE to EXIT", VICTORY_MENU_EXIT);
	//m_victoryMenu->SetBackgroundColor(RGBA(76,175,80));
	m_victoryMenu->SetFontSize(18.0f);
	padding = Vector2(g_theWindow->GetDimensions()) - m_victoryMenu->GetDimensions();
	padding.y -= 256.0f;
	m_victoryMenu->SetExternalPadding(padding / 2.0f);


	// Defeat
	m_defeatEffect = Material::FromShader("FSFX_Saturation");
	m_defeatEffect->SetProperty("END_COLOR", RGBA(244,67,54));

	m_defeatMenu = new Menu();
	m_defeatMenu->Initialize(Vector2(0.0f, 0.0f), true, false);
	m_defeatMenu->SetExternalPadding(Vector2(0.0f, 0.0f));
	m_defeatMenu->AddOption("EXIT", DEFEAT_MENU_EXIT);
	m_defeatMenu->AddOption("CONTINUE", DEFEAT_MENU_CONTINUE);
	m_defeatMenu->SetFontSize(18.0f);
	padding = Vector2(g_theWindow->GetDimensions()) - m_defeatMenu->GetDimensions();
	padding.y -= 256.0f;
	m_defeatMenu->SetExternalPadding(padding / 2.0f);


	// Debug Draw
	//DebugDraw_SetGridVisibility(true);
	//DebugDraw_DrawGrid(30, 10, RGBA(50, 50, 50), RGBA(150, 150, 150));


	// Rendering
	m_forwardRenderPath = new ForwardRenderPath();
	m_renderScene = new RenderScene();


	// Scene
	m_sceneCamera = new OrbitCamera();
	m_sceneCamera->SetColorClearMode(COLOR_CLEAR_MODE_COLOR);
	m_sceneCamera->SetClearColor(RGBA(126,87,194));
	m_sceneCamera->SetViewportNormalizedSize(AABB2D(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f))); 
	Image* skyboxTextures[6] = 
	{
		new Image("Data/Images/GalaxyCube1.png", false),
		new Image("Data/Images/GalaxyCube2.png", false),
		new Image("Data/Images/GalaxyCube3.png", false),
		new Image("Data/Images/GalaxyCube4.png", false),
		new Image("Data/Images/GalaxyCube5.png", false),
		new Image("Data/Images/GalaxyCube6.png", false)
	};
	TextureCube* skybox = new TextureCube((const Image**)skyboxTextures);
	m_sceneCamera->SetClearSkybox(skybox);
	m_sceneCamera->SetDepthClearOptions(true);
	m_sceneCamera->SetColorTarget(0, g_theRenderer->GetDefaultColorTarget());
	m_sceneCamera->SetDepthStencilTarget(g_theRenderer->GetDefaultDepthStencilTarget());
	float cascadeBreakpoints[3] = {20.0f, 70.0f, 200.0f};
	m_sceneCamera->SetCascadeBreakpoints(cascadeBreakpoints);
	//m_sceneCamera->SetProjectionOrtho(1024.0f / 20.0f / 2.0f, -1000.0f, 1000.0f); // For light projection
	m_sceneCamera->SetProjectionPerspective(18.457f, 0.0f, 0.1f, 1000.0f);
	m_sceneCamera->SetCameraOrder(1);
	m_sceneCameraTransform.SetLocalEulerAngles(m_sceneCameraStartEulers);
	m_sceneCameraTransform.SetLocalPosition(m_sceneCameraStartPos);
	m_renderScene->AddCamera(m_sceneCamera);

	// Debug Camera
	m_debugCamera = new OrbitCamera();
	m_debugCamera->SetColorClearMode(COLOR_CLEAR_MODE_COLOR);
	m_debugCamera->SetClearColor(RGBA(126,87,194));
	m_debugCamera->SetViewportNormalizedSize(AABB2D(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f))); 
	m_debugCamera->SetDepthClearOptions(true);
	m_debugCamera->SetColorTarget(0, g_theRenderer->GetDefaultColorTarget());
	m_debugCamera->SetDepthStencilTarget(g_theRenderer->GetDefaultDepthStencilTarget());
	m_debugCamera->SetCascadeBreakpoints(cascadeBreakpoints);
	m_debugCamera->SetProjectionPerspective(18.457f, 0.0f, 0.1f, 1000.0f);
	m_debugCamera->SetCameraOrder(2);
	m_debugCameraTransform.SetLocalEulerAngles(m_sceneCameraStartEulers);
	m_debugCameraTransform.SetLocalPosition(m_sceneCameraStartPos);
	m_debugCamera->ShouldDrawSun(false);
	m_renderScene->AddCamera(m_debugCamera);



	// Ambient Light
	g_theRenderer->SetAmbientLight(RGBA(), 0.1f);


	// Terrain
	m_terrain = new Terrain();
	m_terrain->LoadFromImage("Data/Images/terrainA02.png", AABB2D(Vector2(0.0f, 0.0f), Vector2(1000.0f, 1000.0f)), 0.0f, 0.0f, 1, 256.0f);				// DFS
	//m_terrain->LoadFromImage("Data/Images/terrainA02.png", AABB2D(Vector2(0.0f, 0.0f), Vector2(255.0f, 255.0f)), -5.0f, 10.0f, 1, 128.0f);			// A02
	//m_terrain->LoadFromImage("Data/Images/terrain17.png", AABB2D(Vector2(0.0f, 0.0f), Vector2(63.0f, 63.0f)), /*-5.0f, 10.0f*/0.0f, 0.0f, 16, 32.0f); // Small
	//m_terrain->LoadFromImage("Data/Images/terrain17.png", AABB2D(Vector2(0.0f, 0.0f), Vector2(64.0f, 64.0f)), -2.0f, 5.0f, 4, 32.0f);					// CSM

	// Add each Renderable for the terrain to the render scene
	const std::vector<Renderable*>& terrainRenderables = m_terrain->GetRenderables();
	for (int r = 0; r < (int)terrainRenderables.size(); ++r)
	{
		Renderable* tr = terrainRenderables[r];
		m_renderScene->AddRenderable(tr);

		// Visualize the chunks
		const Mesh* terrainMesh = tr->GetMesh();
		Matrix4 terrainModelTransform = tr->GetModelMatrix();
		for (int j = 0; j < (int)terrainMesh->GetSubMeshCount(); ++j)
		{
			//const SubMesh* submesh = terrainMesh->GetSubMesh(j);
		
			//const AABB3& submeshBounds = submesh->GetBounds();
			//DebugDraw_WireAABB3(1000.0f, (terrainModelTransform * Vector4(mbs.GetCenter(), 1.0f)).XYZ(), (terrainModelTransform * Vector4(mbs.GetHalfDimensions(), 0.0f)).XYZ(), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
			//DebugDraw_WireSphere(1000.0f, mbs.GetCenter(), 0.25f, RGBA(), RGBA(), DEBUG_RENDER_IGNORE_DEPTH);

			// Water
			// Create a water plane that covers each terrain chunk so the water isn't super stretched
			//Renderable* water = new Renderable();
			//Vector3 waterPosition = submeshBounds.GetCenter();
			//waterPosition.y = 1.0f;
			//Vector2 waterHalfDim = submeshBounds.GetHalfDimensions().XZ();
			//Mesh* waterMesh = CreatePlane(waterPosition, waterHalfDim, Vector3::RIGHT, Vector3::FORWARD);
			//water->SetMesh(waterMesh);
			//water->SetMaterial("Water");
			//m_renderScene->AddRenderable(water);
		}

	}


	// The sun
	m_renderScene->SetSun(m_theSunLightDirection, RGBA(), 0.3f);
	//m_theSun = new Light();
	//m_theSun->InitializeDirectionalLight(Vector3(25.0f, 10.0f, 15.0f), Vector3(0.0f, -1.0f, 1.0f).GetNormalized(), RGBA(), 0.6f, Vector3(1.0f, 0.0f, 0.0f));
	//m_theSun->SetDirectionalLightShadowDimensions(20.0f);
	//m_renderScene->AddLight(m_theSun);
	//DebugDraw_WireSphere(1000.0f, m_theSun->GetPosition(), 0.25f, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
	//DebugDraw_LineSegment(1000.0f, m_theSun->GetPosition(), RGBA(), m_theSun->GetPosition() + (m_theSun->GetDirection() * 25.0f), RGBA(), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
	//DebugDraw_Frustum(1000.0f, m_theSun->GetProjectionMatrix(), m_theSun->GetLocalToWorldTransform(), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);


	// Fog
	//m_renderScene->SetFogColor(RGBA(100, 100, 100));
	//m_renderScene->SetFogFactors(0.2f, 0.6f);
	//m_renderScene->SetFogDistances(0.0f, 64.0f);


	// Player
	m_player = new PlayerTank(this);
	m_player->GetTransform().SetWorldPosition(Vector3(5.0f, 0.0f, 5.0f));
	

	// Enemies
	Vector2 minXZ = GetTerrain()->GetExtents().mins + Vector2(10.0f, 10.0f);
	Vector2 maxXZ = GetTerrain()->GetExtents().maxs - Vector2(10.0f, 10.0f);
	for (int i = 0; i < 1; ++i)
	{
		Vector3 randomPosition = Vector3(GetRandomFloatInRange(minXZ.x, maxXZ.x), 0.0f, GetRandomFloatInRange(minXZ.y, maxXZ.y));

		BombEnemySpawner* bes = new BombEnemySpawner(this, 0, 0.25f);
		bes->GetTransform().SetWorldPosition(randomPosition);
		m_bombEnemySpawners.push_back(bes);
	}


	// Background Music
	//m_bgmID = g_theAudio->CreateOrGetSound("Data/Audio/GameplayMusic.mp3");
	//g_theAudio->PlaySound(m_bgmID, true, 0.33f);


	float aspect = 16.0f / 9.0f;
	c1Cam = new Camera();
	c1Cam->SetColorClearMode(COLOR_CLEAR_MODE_COLOR);
	c1Cam->SetClearColor(RGBA(126,87,194));
	c1Cam->SetViewportNormalizedSize(AABB2D(Vector2(0.0f, 0.0f), Vector2(0.2f, 0.2f * aspect))); 
	c1Cam->SetDepthClearOptions(true);
	c1Cam->SetColorTarget(0, g_theRenderer->GetDefaultColorTarget());
	c1Cam->SetDepthStencilTarget(g_theRenderer->GetDefaultDepthStencilTarget());
	c1Cam->SetCameraOrder(3);
	c1Cam->ShouldDrawSun(false);
	c1Cam->SetProjection(m_renderScene->GetSun()->GetProjectionMatrix(0));
	c1Cam->SetLocalToParentMatrix(m_renderScene->GetSun()->GetLocalToWorldTransform(0));
	m_renderScene->AddCamera(c1Cam);


	c2Cam = new Camera();
	c2Cam->SetColorClearMode(COLOR_CLEAR_MODE_COLOR);
	c2Cam->SetClearColor(RGBA(126,87,194));
	c2Cam->SetViewportNormalizedSize(AABB2D(Vector2(0.25f, 0.0f), Vector2(0.45f, 0.2f * aspect))); 
	c2Cam->SetDepthClearOptions(true);
	c2Cam->SetColorTarget(0, g_theRenderer->GetDefaultColorTarget());
	c2Cam->SetDepthStencilTarget(g_theRenderer->GetDefaultDepthStencilTarget());
	c2Cam->SetCameraOrder(3);
	c2Cam->ShouldDrawSun(false);
	c2Cam->SetProjection(m_renderScene->GetSun()->GetProjectionMatrix(1));
	c2Cam->SetLocalToParentMatrix(m_renderScene->GetSun()->GetLocalToWorldTransform(1));
	m_renderScene->AddCamera(c2Cam);

	c3Cam = new Camera();
	c3Cam->SetColorClearMode(COLOR_CLEAR_MODE_COLOR);
	c3Cam->SetClearColor(RGBA(126,87,194));
	c3Cam->SetViewportNormalizedSize(AABB2D(Vector2(0.5f, 0.0f), Vector2(0.7f, 0.2f * aspect))); 
	c3Cam->SetDepthClearOptions(true);
	c3Cam->SetColorTarget(0, g_theRenderer->GetDefaultColorTarget());
	c3Cam->SetDepthStencilTarget(g_theRenderer->GetDefaultDepthStencilTarget());
	c3Cam->SetCameraOrder(3);
	c3Cam->ShouldDrawSun(false);
	c3Cam->SetProjection(m_renderScene->GetSun()->GetProjectionMatrix(2));
	c3Cam->SetLocalToParentMatrix(m_renderScene->GetSun()->GetLocalToWorldTransform(2));
	m_renderScene->AddCamera(c3Cam);


	//DebugDraw_WireSphere(1000.0f, m_sceneCamera->GetPosition(), 0.25f, RGBA(), RGBA(), DEBUG_RENDER_IGNORE_DEPTH);
	//DebugDraw_Frustum(0.0f, m_sceneCamera->GetProjectionForCascade(0), m_sceneCamera->GetLocalToWorldMatrix(), RGBA(0,0,255), RGBA(255,0,255), DEBUG_RENDER_IGNORE_DEPTH);
	//DebugDraw_Frustum(0.0f, m_sceneCamera->GetProjectionForCascade(1), m_sceneCamera->GetLocalToWorldMatrix(), RGBA(0,255,0), RGBA(0,255,0), DEBUG_RENDER_IGNORE_DEPTH);
	//DebugDraw_Frustum(0.0f, m_sceneCamera->GetProjectionForCascade(2), m_sceneCamera->GetLocalToWorldMatrix(), RGBA(255,0,0), RGBA(255,0,0), DEBUG_RENDER_IGNORE_DEPTH);
	//Light* sun = m_renderScene->GetMutableSun();
	//sun->UpdateShadowMaps(m_renderScene, m_sceneCamera);


	// -------------------------------------------------------------------------------------------------------------------------------
	// TEST --------------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------------------
	StaticScene* ss = new StaticScene();
	ss->ImportFile("Data/Models/DFSScene2.fbx");
	m_renderScene->AddStaticScene(ss);
	//
	//
	Vector3 mikuPos = Vector3(10.0f, 0.0f, 0.0f);
	for (int i = 0; i < 60; ++i)
	{
		mikuPos.z += 10.0f;
	
		Renderable* miku = new Renderable();
		miku->SetMesh("Data/Models/SnowMiku.obj");
		miku->SetMaterial("Miku_Base", 0);
		miku->SetMaterial("Miku_Frills", 1);
		miku->SetMaterial("Miku_Body", 2);
		Transform modelTransform = Transform();
		modelTransform.SetLocalScale(Vector3(2.0f, 2.0f, 2.0f));
		modelTransform.SetLocalPosition(mikuPos);
		modelTransform.SetLocalEulerAngles(Vector3(0,0,0));
		miku->SetModelMatrix(modelTransform.GetLocalToWorldMatrix());
		m_renderScene->AddRenderable(miku);
	
		//const Mesh* mikuMesh = miku->GetMesh();
		//for (int j = 0; j < (int)mikuMesh->GetSubMeshCount(); ++j)
		//{
		//	const SubMesh* sm = mikuMesh->GetSubMesh(j);
		//
		//	const AABB3& mbs = sm->GetBounds();
		//	DebugDraw_WireAABB3(1000.0f, (modelTransform.GetLocalToWorldMatrix() * Vector4(mbs.GetCenter(), 1.0f)).XYZ(), (modelTransform.GetLocalToWorldMatrix() * Vector4(mbs.GetHalfDimensions(), 0.0f)).XYZ(), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
		//}
	}

	//Renderable* floor = new Renderable();
	//Mesh* floorMesh = CreateCube(Vector3(0.0f, -1.0f, 0.0f), Vector3(50.0f, 1.0f, 50.0f):);
	//floor->SetMesh(floorMesh);
	//floor->SetMaterial("Terrain");
	//m_renderScene->AddRenderable(floor);


	//box = new Renderable();
	//Mesh* boxMesh = CreateCube(Vector3::ZEROS, Vector3(0.5f, 0.5f, 0.5f));
	//box->SetMesh(boxMesh);
	//box->SetMaterial("Bomber");
	//Transform t;
	//t.SetLocalPosition(Vector3(2.0f, 1.0f, -1.0f));
	//t.SetLocalEulerAngles(Vector3(30.0f, 62.0f, 0.0f));
	//box->SetModelMatrix(t.GetLocalToWorldMatrix());
	//m_renderScene->AddRenderable(box);



	//Renderable* box2 = new Renderable();
	//Mesh* box2Mesh = CreateCube(Vector3::ZEROS, Vector3(0.5f, 0.5f, 0.5f));
	//box2->SetMesh(boxMesh);
	//box2->SetMaterial("Asteroid");
	//Transform t2;
	//t2.SetLocalPosition(Vector3(16.0f, 0.0f, 0.0f));
	//box2->SetModelMatrix(t2.GetLocalToWorldMatrix());
	//m_renderScene->AddRenderable(box2);



	//Light* l1 = new Light();
	////l1->InitializeDirectionalLight(Vector3(5.0f, 5.0f, 0.0f), Vector3(-1.0f, -5.0f, 0.0f).GetNormalized(), RGBA(), 0.6f);
	//l1->InitializeConeLight(Vector3(5.0f, 5.0f, 0.0f), Vector3(-1.0f, -1.0f, 0.0f).GetNormalized(), 15.0f, 30.0f, RGBA(), 25.0f);
	//l1->SetShadowMapUpdateMode(SHADOW_MAP_UPDATE_MODE_INTERVAL, (1.0f / 30.0f));
	////l1->SetShadowCastingState(false);
	////l1->InitializePointLight(Vector3(5.0f, 5.0f, 0.0f), RGBA(), 50.0f);
	////l1->SetShadowCastingState(false);
	//m_renderScene->AddLight(l1);
	//DebugDraw_WireSphere(1000.0f, l1->GetPosition(), 0.25f, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
	//DebugDraw_LineSegment(1000.0f, l1->GetPosition(), RGBA(), l1->GetPosition() + (l1->GetDirection() * 25.0f), RGBA(), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
	

	//Light* l2 = new Light();
	////l2->InitializePointLight(Vector3(-5.0f, 5.0f, 0.0f), RGBA(), 50.0f);
	//l2->InitializeConeLight(Vector3(-5.0f, 5.0f, 0.0f), Vector3(1.0f, -1.0f, 0.0f).GetNormalized(), 15.0f, 30.0f, RGBA(), 25.0f);
	////l2->SetShadowCastingState(false);
	//m_renderScene->AddLight(l2);
	//DebugDraw_WireSphere(1000.0f, l2->GetPosition(), 0.25f, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
	//DebugDraw_LineSegment(1000.0f, l2->GetPosition(), RGBA(), l2->GetPosition() + (l2->GetDirection() * 25.0f), RGBA(), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);


	//Light* l3 = new Light();
	//l3->InitializePointLight(Vector3(0.0f, 2.5f, 5.0f), RGBA(), 50.0f);
	//l3->SetShadowCastingState(false);
	//m_renderScene->AddLight(l3);
	//DebugDraw_WireSphere(1000.0f, Vector3(0.0f, 2.5f, 5.0f), 0.25f, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
	
	
	//Light* l4 = new Light();
	//l4->InitializePointLight(Vector3(0.0f, 2.5f, -5.0f), RGBA(), 50.0f);
	//l4->SetShadowCastingState(false);
	//m_renderScene->AddLight(l4);
	//DebugDraw_WireSphere(1000.0f, Vector3(0.0f, 2.5f, -5.0f), 0.25f, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
	
	
	
	//Matrix4 z45 = Matrix4::CreateRotationAroundZDegrees(45.0f);
	//Matrix4 y45 = Matrix4::CreateRotationAroundYDegrees(45.0f);
	//
	//int i = 5;

	//Ray3 ray = Ray3(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 0.15f, 0.7f));
	//m_terrain->Raycast(ray, 10.0f);


	//Matrix4 tp = Matrix4::CreatePerspectiveProjection(142.0f, 1.0f, 0.1f, 1000.0f);
	//float jy = tp.Jy;
	//float tpDeg = ATanDegrees(1.0f / jy);
	////float ix = 
	//
	//Matrix4 cp = m_sceneCamera->GetProjectionMatrix();
	//Vector4 p1 = cp * Vector4(0.0644f, 0.0333f, 0.1f, 1.0f);
	//Vector4 p2 = cp * Vector4(12.895f, 6.675f,  20.0f, 1.0f);
	//Vector4 p3 = cp * Vector4(51.580f, 26.7f,   80.0f, 1.0f);
	//Vector4 p4 = cp * Vector4(257.901f, 133.5f,  400.0f, 1.0f);
	//
	//p1 /= p1.w;
	//p2 /= p2.w;
	//p3 /= p3.w;
	//p4 /= p4.w;
	//
	//
	//
	//
	//Vector4 frustumPoints[8] = {
	//	Vector4(-1.0f, -1.0f, -1.0f, 1.0f),
	//	Vector4(1.0f, -1.0f, -1.0f, 1.0f),
	//	Vector4(1.0f, 1.0f, -1.0f, 1.0f),
	//	Vector4(-1.0f, 1.0f, -1.0f, 1.0f),
	//
	//	Vector4(-1.0f, -1.0f, 1.0f, 1.0f),
	//	Vector4(1.0f, -1.0f, 1.0f, 1.0f),
	//	Vector4(1.0f, 1.0f, 1.0f, 1.0f),
	//	Vector4(-1.0f, 1.0f, 1.0f, 1.0f),
	//};
	//
	//Matrix4 M = m_sceneCamera->GetProjectionMatrix(); // * m_sceneCamera->GetWorldToLocalMatrix();
	//M.Invert();
	//for (int i = 0; i < 8; ++i)
	//{
	//	frustumPoints[i] = M * frustumPoints[i];
	//	frustumPoints[i] /= frustumPoints[i].w;
	//}
	//
	//float x1t = RangeMapFloat(0.1f, 0.1f, 1000.0f, 0.0f, 1.0f);
	//float x2t = RangeMapFloat(20.0f, 0.1f, 1000.0f, 0.0f, 1.0f);
	//float x3t = RangeMapFloat(80.0f, 0.1f, 1000.0f, 0.0f, 1.0f);
	//float x4t = RangeMapFloat(400.0f, 0.1f, 1000.0f, 0.0f, 1.0f);
	//
	//
	//Vector3 x1 = Interpolate(frustumPoints[2].XYZ(), frustumPoints[6].XYZ(), x1t);
	//Vector3 x2 = Interpolate(frustumPoints[2].XYZ(), frustumPoints[6].XYZ(), x2t);
	//Vector3 x3 = Interpolate(frustumPoints[2].XYZ(), frustumPoints[6].XYZ(), x3t);
	//Vector3 x4 = Interpolate(frustumPoints[2].XYZ(), frustumPoints[6].XYZ(), x4t);
	//
	//
	//int i = 5;
	//Camera* tc = new OrbitCamera();
	//tc->SetColorClearMode(COLOR_CLEAR_MODE_COLOR);
	//tc->SetClearColor(RGBA(126,87,194));
	//tc->SetViewportNormalizedSize(AABB2D(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f))); 
	//tc->SetClearSkybox(skybox);
	//tc->SetDepthClearOptions(true);
	//tc->SetColorTarget(g_theRenderer->GetDefaultColorTarget());
	//tc->SetDepthStencilTarget(g_theRenderer->GetDefaultDepthStencilTarget());
	//float tcdepths[3] = {5.0f, 15.0f, 40.0f};
	//tc->SetCascadeDepths(tcdepths);
	//tc->SetProjectionPerspective(18.457f, 0.0f, 0.1f, 1000.0f);
	//tc->SetCameraOrder(1);
	//
	//
	//Light* ts = new Light();
	//ts->InitializeSunLight(Vector3(-1.0f, 0.0f, 3.0f).GetNormalized(), RGBA(), 0.6f);
	//
	//ts->UpdateShadowMaps(m_renderScene, tc);
	//
	//
	//DebugDraw_Frustum(1000.0f, tc->GetProjectionForCascade(0), tc->GetLocalToWorldMatrix(), RGBA(76,175,80), RGBA(76,175,80), DEBUG_RENDER_USE_DEPTH);
	//DebugDraw_Frustum(1000.0f, tc->GetProjectionForCascade(1), tc->GetLocalToWorldMatrix(), RGBA(255,152,0), RGBA(255,152,0), DEBUG_RENDER_USE_DEPTH);
	//DebugDraw_Frustum(1000.0f, tc->GetProjectionForCascade(2), tc->GetLocalToWorldMatrix(), RGBA(244,67,54), RGBA(244,67,54), DEBUG_RENDER_USE_DEPTH);
	
	//Light* mts = m_renderScene->GetMutableSun();
	//DebugDraw_Frustum(1000.0f, m_sceneCamera->GetProjectionForCascade(0), m_sceneCamera->GetLocalToWorldMatrix(), RGBA(76,175,80), RGBA(76,175,80), DEBUG_RENDER_USE_DEPTH);
	//DebugDraw_Frustum(1000.0f, m_sceneCamera->GetProjectionForCascade(1), m_sceneCamera->GetLocalToWorldMatrix(), RGBA(255,152,0), RGBA(255,152,0), DEBUG_RENDER_USE_DEPTH);
	//DebugDraw_Frustum(1000.0f, m_sceneCamera->GetProjectionForCascade(2), m_sceneCamera->GetLocalToWorldMatrix(), RGBA(244,67,54), RGBA(244,67,54), DEBUG_RENDER_USE_DEPTH);
	//mts->UpdateShadowMaps(m_renderScene, m_sceneCamera);
	//
	//Matrix4 cb = mts->GetCascadeBasis(0);
	//Matrix4 cbi = cb;
	//cbi.Invert();
	//
	//Matrix4 cp = mts->GetCascadeProjection(0);
	//Matrix4 vp = cp * cbi;
	//
	//Matrix4 vp2 = mts->GetViewProjectionMatrix(0);


	//DebugDraw_WireSphere(1000.0f, Vector3(20.0f, 5.0f, 20.0f), 5.0f, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);

	//m_sceneCamera->SetLocalToParentMatrix(mts->GetCascadeBasis(0));
	//m_sceneCamera->SetProjection(mts->GetCascadeProjection(0));
	//m_sceneCamera->OverrideViewportBaseResolution(true, IntVector2(1024, 1024));


	//for (int i = 0; i < 25; ++i)
	//{
	//	Light* l = new Light();
	//
	//	float rx = GetRandomFloatInRange(0.0f, 63.0f);
	//	float ry = GetRandomFloatInRange(5.0f, 10.0f);
	//	float rz = GetRandomFloatInRange(0.0f, 63.0f);
	//	Vector3 rPos = Vector3(rx, ry, rz);
	//
	//	l->InitializePointLight(rPos, RGBA(), 50.0f);
	//	l->SetShadowCastingState(false);
	//	m_renderScene->AddLight(l);
	//	DebugDraw_WireSphere(1000.0f, rPos, 0.25f, RGBA(0,0,255), RGBA(0,0,255), DEBUG_RENDER_IGNORE_DEPTH);
	//}
}



void PlayState::Destroy()
{
	DebugDraw_ClearCommands();

	// Camera
	delete m_fullScreenCamera;
	m_fullScreenCamera = nullptr;

	// Pause Menu
	delete m_pauseMenu;
	m_pauseMenu = nullptr;

	// Pause Effect
	delete m_pauseEffect;
	m_pauseEffect = nullptr;

	// Scene
	delete m_sceneCamera;
	m_sceneCamera = nullptr;

	delete m_theSun;
	m_theSun = nullptr;

	delete m_player;
	m_player = nullptr;

	// Rendering
	delete m_forwardRenderPath;
	m_forwardRenderPath = nullptr;

	delete m_renderScene;
	m_renderScene = nullptr;

	g_theAudio->StopSound(m_bgmID);
}


static bool updatePlayer = true;
static bool showDebug = false;
static int  cameraLayerShown = 2;
static int  cameraLayerHidden = 0;
void PlayState::Update(float deltaSeconds)
{
	PROFILE_SCOPE_FUNCTION();

	UNUSED(deltaSeconds);
 
 	Light* spotlight = m_theSun;
 	std::vector<Light*> lights = m_renderScene->GetMutableLights();
 	for (int i = 0; i < (int)lights.size(); ++i)
 	{
 		if (lights[i]->GetLightType() == LIGHT_TYPE_CONE)
 		{
 			spotlight = lights[i];
 		}
 	}
 	if (g_theInput->WasKeyJustPressed('B'))
 	{
 		spotlight->SetShadowMapUpdateMode(SHADOW_MAP_UPDATE_MODE_ALWAYS);
 	}
 	if (g_theInput->WasKeyJustPressed('N'))
 	{
 		spotlight->SetShadowMapUpdateMode(SHADOW_MAP_UPDATE_MODE_INTERVAL, (1.0f / 20.0f));
 	}
 	if (g_theInput->WasKeyJustPressed('M'))
 	{
 		spotlight->SetShadowMapUpdateMode(SHADOW_MAP_UPDATE_MODE_NEVER);
 	}

	if (g_theInput->WasKeyJustPressed('C'))
	{
		showDebug = !showDebug;

		if (showDebug)
		{
			c1Cam->SetCameraOrder(cameraLayerShown);
			c2Cam->SetCameraOrder(cameraLayerShown);
			c3Cam->SetCameraOrder(cameraLayerShown);
		}
		else
		{
			c1Cam->SetCameraOrder(cameraLayerHidden);
			c2Cam->SetCameraOrder(cameraLayerHidden);
			c3Cam->SetCameraOrder(cameraLayerHidden);
		}
	}


	DebugDraw_Frustum(0.0f, m_sceneCamera->GetProjectionForCascade(0), m_sceneCamera->GetLocalToWorldMatrix(), RGBA(0,0,255), RGBA(255,0,255), DEBUG_RENDER_IGNORE_DEPTH);
	DebugDraw_Frustum(0.0f, m_sceneCamera->GetProjectionForCascade(1), m_sceneCamera->GetLocalToWorldMatrix(), RGBA(0,255,0), RGBA(0,255,0), DEBUG_RENDER_IGNORE_DEPTH);
	DebugDraw_Frustum(0.0f, m_sceneCamera->GetProjectionForCascade(2), m_sceneCamera->GetLocalToWorldMatrix(), RGBA(255,0,0), RGBA(255,0,0), DEBUG_RENDER_IGNORE_DEPTH);

	c1Cam->SetProjection(m_renderScene->GetSun()->GetProjectionMatrix(0));
	c1Cam->SetLocalToParentMatrix(m_renderScene->GetSun()->GetLocalToWorldTransform(0));
	c2Cam->SetProjection(m_renderScene->GetSun()->GetProjectionMatrix(1));
	c2Cam->SetLocalToParentMatrix(m_renderScene->GetSun()->GetLocalToWorldTransform(1));
	c3Cam->SetProjection(m_renderScene->GetSun()->GetProjectionMatrix(2));
	c3Cam->SetLocalToParentMatrix(m_renderScene->GetSun()->GetLocalToWorldTransform(2));


	// Handle pause
	if (IsPaused())
	{
		ProcessStateInput_Paused();
		UpdatePauseState();
	}
	else if (HasWon())
	{
		ProcessStateInput_Victory();
		UpdateVictoryState();
	}
	else if (HasLost())
	{
		if(!m_hasLost)
		{
			SoundID playerDeathSID = g_theAudio->CreateOrGetSound("Data/Audio/PlayerDied.wav");
			g_theAudio->PlaySound(playerDeathSID, false);
		}

		m_hasLost = true;

		ProcessStateInput_Defeat();
		UpdateDefeatState();
	}
	else
	{
		ProcessStateInput();

		// Update Scene
		if (!g_theDevConsole->IsOpen())
		{
			if (g_theInput->WasKeyJustPressed('P'))
			{
				updatePlayer = !updatePlayer;
				m_debugCameraTransform = m_sceneCameraTransform;
			}

			if (updatePlayer)
			{
				m_player->Update(deltaSeconds);
				UpdateCameraForPlayer(deltaSeconds);
				m_debugCamera->SetCameraOrder(0);
			}
			else
			{
				m_debugCamera->SetCameraOrder(2);
				UpdateFreeCamera(deltaSeconds);
			}

			UpdateEnemies(deltaSeconds);
		}

		HandleAllCollisions();
		RemoveAllDestroyableEntities();
	}

	HasWon();
}



void PlayState::UpdatePauseState()
{
	if (m_shouldUnpause && (m_pauseFadeTime == 0.0f))
	{
		m_isPaused = false;
		m_shouldUnpause = false;
		m_pauseFadeTime = 0.0f;
		m_pauseDirection = 1.0f;
		m_pauseMenu->HideMenu();
		g_theGameClock->SetPauseState(false);
	}
	else
	{
		m_pauseFadeTime += (m_pauseDirection * g_theMasterClock->GetDeltaSeconds());
		m_pauseFadeTime = ClampFloat(m_pauseFadeTime, 0.0f, PAUSE_FADE_MAX_TIME);
	}
}



void PlayState::UpdateVictoryState()
{
	// Update the fade
	m_victoryFadeTime += g_theMasterClock->GetDeltaSeconds();
	m_victoryFadeTime = ClampFloat(m_victoryFadeTime, 0.0f, VICTORY_FADE_MAX_TIME);
}


void PlayState::UpdateDefeatState()
{
	// If we are fading back into the scene, and we are fully faded back in
	if (m_defeatFadeDirection < 0.0f && m_defeatFadeTime <= 0.0f)
	{
		m_hasLost = false;
		m_defeatFadeDirection = 1.0f;
		GetPlayer()->GetHealthPool()->SetHealth(GetPlayer()->GetHealthPool()->GetMaxHealth());
		GetPlayer()->GetTransform().SetWorldPosition(GetSafeSpawnPositionForPlayer());
	}


	// Update the fade
	m_defeatFadeTime += (m_defeatFadeDirection * g_theMasterClock->GetDeltaSeconds());
	m_defeatFadeTime = ClampFloat(m_defeatFadeTime, 0.0f, DEFEAT_FADE_MAX_TIME);
}


Vector3 cameraEulers = Vector3(0.0f, 0.0f, 0.0f);
void PlayState::UpdateCameraForPlayer(float deltaSeconds)
{
	// Rotation
	if (!g_theProfiler->IsCapturingMouse())
	{
		Vector3 eulerAnglesDegrees = cameraEulers;
		IntVector2 mouseDelta = g_theInput->GetMouseDelta();
		eulerAnglesDegrees.x += -mouseDelta.y * m_sceneCameraRotSpeed * deltaSeconds;
		eulerAnglesDegrees.y += mouseDelta.x * m_sceneCameraRotSpeed * deltaSeconds;
		eulerAnglesDegrees.x = ClampFloat(eulerAnglesDegrees.x, m_sceneCameraXRange.min, m_sceneCameraXRange.max);
		cameraEulers = eulerAnglesDegrees;
		m_sceneCameraTransform.SetLocalEulerAngles(eulerAnglesDegrees);
	}

	// Positioning
	m_sceneCameraTransform.SetWorldPosition(GetPlayer()->GetTransform().GetWorldPosition());
	m_sceneCameraTransform.TranslateLocal(Vector3(0.0f, 2.0f, -7.0f));

	m_sceneCamera->SetLocalToParentMatrix(m_sceneCameraTransform.GetLocalToWorldMatrix());
}



void PlayState::UpdateFreeCamera(float deltaSeconds)
{
	// Rotation
	if(!g_theProfiler->IsCapturingMouse())
	{
		Vector3 eulerAnglesDegrees = m_debugCameraTransform.GetLocalEulerAngles();
		IntVector2 mouseDelta = g_theInput->GetMouseDelta();
		eulerAnglesDegrees.x += -mouseDelta.y * m_debugCameraRotSpeed * deltaSeconds;
		eulerAnglesDegrees.y += mouseDelta.x * m_debugCameraRotSpeed * deltaSeconds;
		eulerAnglesDegrees.x = ClampFloat(eulerAnglesDegrees.x, m_debugCameraXRange.min, m_debugCameraXRange.max);
		m_debugCameraTransform.SetLocalEulerAngles(eulerAnglesDegrees);
	}

	// Translation
	Vector3 movementDirection = Vector3::ZEROS;
	if (g_theInput->IsKeyPressed('W'))
	{
		movementDirection += Vector3::FORWARD;
	}
	if (g_theInput->IsKeyPressed('S'))
	{
		movementDirection += -Vector3::FORWARD;
	}
	if (g_theInput->IsKeyPressed('D'))
	{
		movementDirection += Vector3::RIGHT;
	}
	if (g_theInput->IsKeyPressed('A'))
	{
		movementDirection += -Vector3::RIGHT;
	}
	if (g_theInput->IsKeyPressed('E'))
	{
		movementDirection += Vector3::UP;
	}
	if (g_theInput->IsKeyPressed('Q'))
	{
		movementDirection += -Vector3::UP;
	}

	// If we want to move
	if (movementDirection.GetLengthSquared() > 0.0f)
	{
		float shiftMutliplier = 1.0f;
		if (g_theInput->IsKeyPressed(InputSystem::KEY_SHIFT))
		{
			shiftMutliplier = 10.0f;
		}

		movementDirection.NormalizeAndGetLength();
		m_debugCameraTransform.TranslateLocal(movementDirection * deltaSeconds* m_debugCameraMoveSpeed * shiftMutliplier);
	}

	m_debugCamera->SetLocalToParentMatrix(m_debugCameraTransform.GetLocalToWorldMatrix());
}



void PlayState::UpdateEnemies(float deltaSeconds)
{
	UpdateBullets(deltaSeconds);
	UpdateBombEnemySpawners(deltaSeconds);
	UpdateBombEnemies(deltaSeconds);
}



void PlayState::UpdateBombEnemies(float deltaSeconds)
{
	for (int i = 0; i < (int)m_bombEnemies.size(); ++i)
	{
		m_bombEnemies[i]->Update(deltaSeconds);
	}
}



void PlayState::UpdateBombEnemySpawners(float deltaSeconds)
{
	for (int i = 0; i < (int)m_bombEnemySpawners.size(); ++i)
	{
		m_bombEnemySpawners[i]->Update(deltaSeconds);
	}
}



void PlayState::UpdateBullets(float deltaSeconds)
{
	for (int i = 0; i < (int)m_bullets.size(); ++i)
	{
		m_bullets[i]->Update(deltaSeconds);
	}
}



void PlayState::Render3D() const
{
	PROFILE_SCOPE_FUNCTION();

	m_forwardRenderPath->Render(m_renderScene);
	m_forwardRenderPath->ClearAllTemporaryRenderables();

	Material* gammaEncoder = Material::FromShader("FSFX_GammaEncoder");
	g_theRenderer->ApplyEffect(gammaEncoder);
	g_theRenderer->FinishEffects();
	delete gammaEncoder;

	DebugDraw_2DText(0.0f, Vector2(1400.0f, 882.0f), 18.0f, RGBA(), RGBA(), "FPS: %.0f", 1.0f / g_theMasterClock->GetDeltaSeconds());
}



void PlayState::Render2D() const
{
	PROFILE_SCOPE_FUNCTION();

	g_theRenderer->BindCamera(m_fullScreenCamera);


	if (showDebug)
	{
		float step = 200.0f + 200.0f;
		DebugDraw_2DQuad(0.0f, AABB2D(Vector2(0.0f + (step * 0),	320.0f),	Vector2(200.0f + (step * 0),	520.0f)), m_renderScene->GetMutableSun()->GetShadowMapForDrawing(0));
		DebugDraw_2DQuad(0.0f, AABB2D(Vector2(0.0f + (step * 1),	320.0f),	Vector2(200.0f + (step * 1),	520.0f)), m_renderScene->GetMutableSun()->GetShadowMapForDrawing(1));
		DebugDraw_2DQuad(0.0f, AABB2D(Vector2(0.0f + (step * 2),	320.0f),	Vector2(200.0f + (step * 2),	520.0f)), m_renderScene->GetMutableSun()->GetShadowMapForDrawing(2));
	}

	/*
	// UI
	// Font constants
	const BitmapFont* font = BitmapFont::CreateOrGet("Data/Fonts/SquirrelFixedFont.png");
	float fontSize = 27.0f;
	Vector2 startPosition = Vector2(16.0f, 40.0f);

	// Draw health header
	std::string healthTextHeader = "Health: ";
	g_theRenderer->DrawText2D(startPosition, healthTextHeader, fontSize, RGBA(), 1.0f, font);

	// Draw health value
	float currentHealth = GetPlayer()->GetHealthPool()->GetHealth();
	float maxHealth = GetPlayer()->GetHealthPool()->GetMaxHealth();
	std::string healthTextValue = Stringf("%.0f/%.0f", currentHealth, maxHealth);
	float healthTextHeaderWidth = font->GetStringWidth(healthTextHeader, fontSize, 1.0f);
	RGBA healthTextValueColor = Interpolate(RGBA(255,0,0), RGBA(0,255,0), currentHealth / maxHealth);
	g_theRenderer->DrawText2D(startPosition + Vector2(healthTextHeaderWidth, 0.0f), healthTextValue, fontSize, healthTextValueColor, 1.0f, font);


	// Draw Enemy Count
	startPosition.y += 27.0f;
	std::string enemyTextHeader = "Enemy Count: ";
	g_theRenderer->DrawText2D(startPosition, enemyTextHeader, fontSize, RGBA(), 1.0f, font);

	// 
	float enemyTextHeaderWidth = font->GetStringWidth(enemyTextHeader, fontSize, 1.0f);
	std::string enemyTextValue = Stringf("%i", m_bombEnemies.size() + m_bombEnemySpawners.size());
	g_theRenderer->DrawText2D(startPosition + Vector2(enemyTextHeaderWidth, 0.0f), enemyTextValue, fontSize, RGBA(), 1.0f, font);
	*/


	// Overlays
	if (IsPaused())
	{
		RenderPauseOverlay();
	}
	if (HasWon())
	{
		RenderVictoryOverlay();
	}
	if (HasLost())
	{
		RenderDefeatOverlay();
	}
}



void PlayState::RenderPauseOverlay() const
{
	// Apply the desaturation effect
	GUARANTEE_OR_DIE(PAUSE_FADE_MAX_TIME != 0.0f, "Dont set the pause fade time to 0.0f");
	float percentageComplete = m_pauseFadeTime / PAUSE_FADE_MAX_TIME;
	m_pauseEffect->SetProperty("PERCENT_COMPLETE", percentageComplete);
	g_theRenderer->ApplyEffect(m_pauseEffect);
	g_theRenderer->FinishEffects();

	g_theRenderer->BindCamera(m_fullScreenCamera);
	g_theRenderer->BindShader(Shader::Get("Default_2D"));
	RGBA drawColor = Interpolate(RGBA(255, 255, 255, 0), RGBA(), percentageComplete);
	g_theRenderer->DrawTextInBox2D(AABB2D(Vector2(0.0f, 0.0f), Vector2(g_theWindow->GetDimensions())), "Paused", 64.0f, drawColor, 1.0f, BitmapFont::CreateOrGet("Data/Fonts/SquirrelFixedFont.png"), Vector2(0.5f, 0.5f), TEXT_DRAW_OVERRUN);
	m_pauseMenu->Render(); 
}



void PlayState::RenderVictoryOverlay() const
{
	// Apply the Saturation effect
	GUARANTEE_OR_DIE(VICTORY_FADE_MAX_TIME != 0.0f, "Dont set the victory fade time to 0.0f");
	float percentageComplete = m_victoryFadeTime / VICTORY_FADE_MAX_TIME;
	m_victoryEffect->SetProperty("PERCENT_COMPLETE", percentageComplete);
	g_theRenderer->ApplyEffect(m_victoryEffect);
	g_theRenderer->FinishEffects();

	g_theRenderer->BindCamera(m_fullScreenCamera);
	g_theRenderer->BindShader(Shader::Get("Default_2D"));
	RGBA drawColor = Interpolate(RGBA(255, 255, 255, 0), RGBA(), percentageComplete);
	g_theRenderer->DrawTextInBox2D(AABB2D(Vector2(0.0f, 0.0f), Vector2(g_theWindow->GetDimensions())), "Victory", 64.0f, drawColor, 1.0f, BitmapFont::CreateOrGet("Data/Fonts/SquirrelFixedFont.png"), Vector2(0.5f, 0.5f), TEXT_DRAW_OVERRUN);

	if(m_victoryFadeTime >= VICTORY_FADE_MAX_TIME)
	{
		m_victoryMenu->Render(); 
	}
}



void PlayState::RenderDefeatOverlay() const
{
	// Apply the Saturation effect
	GUARANTEE_OR_DIE(DEFEAT_FADE_MAX_TIME != 0.0f, "Dont set the defeat fade time to 0.0f");
	float percentageComplete = m_defeatFadeTime /DEFEAT_FADE_MAX_TIME;
	float shaderPercentage = RangeMapFloat(percentageComplete, 0.0f, 1.0f, 0.0f, 0.4f);
	m_defeatEffect->SetProperty("PERCENT_COMPLETE", shaderPercentage);
	g_theRenderer->ApplyEffect(m_defeatEffect);
	g_theRenderer->FinishEffects();

	g_theRenderer->BindCamera(m_fullScreenCamera);
	g_theRenderer->BindShader(Shader::Get("Default_2D"));
	RGBA drawColor = Interpolate(RGBA(255, 255, 255, 0), RGBA(), percentageComplete);
	g_theRenderer->DrawTextInBox2D(AABB2D(Vector2(0.0f, 0.0f), Vector2(g_theWindow->GetDimensions())), "Defeat", 64.0f, drawColor, 1.0f, BitmapFont::CreateOrGet("Data/Fonts/SquirrelFixedFont.png"), Vector2(0.5f, 0.5f), TEXT_DRAW_OVERRUN);

	if(m_defeatFadeTime >= DEFEAT_FADE_MAX_TIME)
	{
		m_defeatMenu->Render(); 
	}
}



RenderScene* PlayState::GetRenderScene() const
{
	return m_renderScene;
}



ForwardRenderPath* PlayState::GetForwardRenderPass() const
{
	return m_forwardRenderPath;
}



//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
bool PlayState::IsPaused() const
{
	return m_isPaused;
}


//------------------------------------------------------------------------
void PlayState::Pause()
{
	m_isPaused = true;
	m_pauseMenu->ShowMenu();
	g_theGameClock->SetPauseState(true);
}


//------------------------------------------------------------------------
void PlayState::Unpause()
{
	m_shouldUnpause = true;
	m_pauseDirection = -1.0f;
}


//------------------------------------------------------------------------
void PlayState::TogglePause()
{
	if(IsPaused())
	{
		SetPauseState(false);
	}
	else
	{
		SetPauseState(true);
	}
}


//------------------------------------------------------------------------
void PlayState::SetPauseState(bool state)
{
	// Short Circuit
	if (m_isPaused == state)
	{
		return;
	}

	if (state == true)
	{
		Pause();
	}
	else
	{
		Unpause();
	}
}


//------------------------------------------------------------------------
bool PlayState::HasWon() const
{
	bool hasWon = false;

	int bombCount = GetNumEntitiesWithTags(m_bombEnemies, "RedTeam");
	int spawnerCount = GetNumEntitiesWithTags(m_bombEnemySpawners, "RedTeam");

	// If there are no enemies we win!
	int enemyCount = bombCount + spawnerCount;
	if (enemyCount == 0)
	{
		hasWon = true;
	}

	return hasWon;
}



int	PlayState::GetNumEntitiesWithTags(const std::vector<Entity*>& entities, const std::string& tags) const
{
	int count = 0;

	for (int i = 0; i < (int)entities.size(); ++i)
	{
		if (entities[i]->GetTags().HasCSV(tags))
		{
			++count;
		}
	}

	return count;
}


//------------------------------------------------------------------------
bool PlayState::HasLost() const
{
	bool hasLost = false;

	if (m_hasLost || GetPlayer()->GetHealthPool()->GetHealth() <= 0.0f)
	{
		hasLost = true;
	}

	return hasLost;
}



//------------------------------------------------------------------------
void PlayState::ProcessStateInput()
{
	if (!g_theDevConsole->IsOpen())
	{
		XboxController& controller = g_theInput->GetController(CONTROLLER_NUMBER_1);
		if((g_theInput->WasKeyJustPressed(InputSystem::KEY_ESCAPE) || (controller.IsConnected() && controller.WasButtonJustPressed(XboxController::BUTTON_BACK))) 
			&& !g_theDevConsole->WasJustClosed() )
		{
			GetOwner()->StartTransitionToState(GAME_STATE_ATTRACT);
			GetOwner()->FinishTransition();
		}
		if(g_theInput->WasKeyJustPressed(InputSystem::KEY_F5) || (controller.IsConnected() && controller.WasButtonJustPressed(XboxController::BUTTON_START)))
		{
			TogglePause();
		}
	}
}


//------------------------------------------------------------------------
void PlayState::ProcessStateInput_Paused()
{
	if (!g_theDevConsole->IsOpen() && !g_theDevConsole->WasJustClosed())
	{
		m_pauseMenu->ProcessInput();
		unsigned int frameSelection = m_pauseMenu->GetFrameSelection();

		XboxController& controller = g_theInput->GetController(CONTROLLER_NUMBER_1);
		if( (frameSelection == PAUSE_MENU_EXIT) )
		{
			GetOwner()->StartTransitionToState(GAME_STATE_ATTRACT);
			GetOwner()->FinishTransition();
		}
		if( g_theInput->WasKeyJustPressed(InputSystem::KEY_ESCAPE) ||
			(controller.IsConnected() && controller.WasButtonJustPressed(XboxController::BUTTON_B)) ||
			(frameSelection == PAUSE_MENU_RESUME) )
		{
			Unpause();
		}
	}
}


//------------------------------------------------------------------------
void PlayState::ProcessStateInput_Victory()
{
	if (m_victoryFadeTime >= VICTORY_FADE_MAX_TIME)
	{
		m_victoryMenu->ProcessInput();
		unsigned int frameSelection = m_victoryMenu->GetFrameSelection();

		XboxController& controller = g_theInput->GetController(CONTROLLER_NUMBER_1);
		if( (frameSelection == VICTORY_MENU_EXIT) ||
			g_theInput->WasKeyJustPressed(InputSystem::KEY_ESCAPE) ||
			(controller.IsConnected() && controller.WasButtonJustPressed(XboxController::BUTTON_B)) 
		  )
		{
			GetOwner()->StartTransitionToState(GAME_STATE_ATTRACT);
			GetOwner()->FinishTransition();
		}
	}
}


//------------------------------------------------------------------------
void PlayState::ProcessStateInput_Defeat()
{
	if (m_defeatFadeTime >= DEFEAT_FADE_MAX_TIME)
	{
		m_defeatMenu->ProcessInput();
		unsigned int frameSelection = m_defeatMenu->GetFrameSelection();

		if( (frameSelection == DEFEAT_MENU_EXIT) )
		{
			GetOwner()->StartTransitionToState(GAME_STATE_ATTRACT);
			GetOwner()->FinishTransition();
		}
		else if ( frameSelection == DEFEAT_MENU_CONTINUE)
		{
			m_defeatFadeDirection = -1.0f;
		}
	}
}


//------------------------------------------------------------------------
const Terrain* PlayState::GetTerrain() const
{
	return m_terrain;
}



Camera* PlayState::GetSceneCamera() const
{
	return m_sceneCamera;
}



PlayerTank* PlayState::GetPlayer() const
{
	return m_player;
}



const std::vector<Entity*>& PlayState::GetBombEnemies() const
{
	return m_bombEnemies;
}



bool PlayState::HasEntityWithPointer(void* p) const
{
	// Player
	if (m_player == p)
	{
		return true;
	}
	
	// Bombs
	for (int i = 0; i < (int)m_bombEnemies.size(); ++i)
	{
		if (m_bombEnemies[i] == p)
		{
			return true;
		}
	}

	// Bomb Spawners
	for (int i = 0; i < (int)m_bombEnemySpawners.size(); ++i)
	{
		if (m_bombEnemySpawners[i] == p)
		{
			return true;
		}
	}

	return false;
}



BombEnemy* PlayState::SpawnBombEnemy(const Vector2& xzPosition)
{
	BombEnemy* be = new BombEnemy(this);

	float height = GetTerrain()->GetHeight(xzPosition);
	be->GetTransform().SetWorldPosition(Vector3(xzPosition.x, height, xzPosition.y));
	m_bombEnemies.push_back(be);

	return be;
}



Bullet* PlayState::SpawnBullet(const Vector3& startPosition, const Vector3& direction, float speed, float damage, const std::string& damageConditions, float damageRadius, float lifetimeSeconds, float physicalRadius)
{
	Bullet* bullet = new Bullet(this, startPosition, direction, speed, damage, damageConditions, damageRadius, lifetimeSeconds, physicalRadius);

	m_bullets.push_back(bullet);

	return bullet;
}



void PlayState::RemoveAllDestroyableEntities()
{
	RemoveDestroyableEntities(m_bullets);
	RemoveDestroyableEntities(m_bombEnemies);
	RemoveDestroyableEntities(m_bombEnemySpawners);
}



void PlayState::RemoveDestroyableEntities(std::vector<Entity*>& entities)
{
	for (int i = 0; i < (int)entities.size();)
	{
		if(entities[i]->CanBeDestroyed())
		{
			delete entities[i];
			entities[i] = entities[entities.size() - 1];
			entities.erase(entities.begin() + entities.size() - 1);
		}
		else
		{
			++i;
		}
	}
}



void PlayState::HandleAllCollisions()
{
	PROFILE_SCOPE_FUNCTION();

	// Cache player collision data
	Entity* player = GetPlayer();
	Vector3 playerPosition = player->GetTransform().GetWorldPosition();
	float   playerCollisionRadius = player->GetCollisionRadius();


	// Bullets
	for (int i = 0; i < (int)m_bullets.size(); ++i)
	{
		Entity* bullet = m_bullets[i];
		Vector3 bulletPosition = bullet->GetTransform().GetWorldPosition();
		float   bulletCollisionRadius = bullet->GetCollisionRadius();

		// Bullet vs BombEnemy
		for (int j = 0; j < (int)m_bombEnemies.size(); ++j)
		{
			Entity* bombEnemy = m_bombEnemies[j];
			Vector3 bombEnemyPosition = bombEnemy->GetTransform().GetWorldPosition();
			float   bombEnemyCollisionRadius = bombEnemy->GetCollisionRadius();

			bool collided = DoSphereSphereCollision(bulletPosition, bulletCollisionRadius, bombEnemyPosition, bombEnemyCollisionRadius);
			if (collided)
			{
				bullet->OnCollision(bombEnemy);
				bombEnemy->OnCollision(bullet);
			}
		}

		// Bullet vs BombSpawner
		for (int j = 0; j < (int)m_bombEnemySpawners.size(); ++j)
		{
			Entity* spawner = m_bombEnemySpawners[j];
			Vector3 spawnerPosition = spawner->GetTransform().GetWorldPosition();
			float   spawnerCollisionRadius = spawner->GetCollisionRadius();

			bool collided = DoSphereSphereCollision(bulletPosition, bulletCollisionRadius, spawnerPosition, spawnerCollisionRadius);
			if (collided)
			{
				bullet->OnCollision(spawner);
				spawner->OnCollision(bullet);
			}
		}
	}


	// Bomb Enemies
	for (int i = 0; i < (int)m_bombEnemies.size(); ++i)
	{
		Entity* bombEnemy = m_bombEnemies[i];
		Vector3 bombEnemyPosition = bombEnemy->GetTransform().GetWorldPosition();
		float   bombEnemyCollisionRadius = bombEnemy->GetCollisionRadius();

		bool collided = DoSphereSphereCollision(bombEnemyPosition, bombEnemyCollisionRadius, playerPosition, playerCollisionRadius);
		if (collided)
		{
			bombEnemy->OnCollision(player);
			player->OnCollision(bombEnemy);
		}
	}
}



bool PlayState::DoSphereSphereCollision(const Vector3& sphere1Position, float sphere1Radius, const Vector3& sphere2Position, float sphere2Radius)
{
	bool collided = true;

	float distanceBetween = (sphere2Position - sphere1Position).GetLength();
	float combinedRadius = sphere1Radius + sphere2Radius;
	if (distanceBetween > combinedRadius)
	{
		collided = false;
	}

	return collided;
}



Vector3 PlayState::GetSafeSpawnPositionForPlayer() const
{
	Vector3 spawnPosition;

	bool isSafe = false;
	AABB2D terrainExtents = GetTerrain()->GetExtents();
	while(!isSafe)
	{
		// Get a random position on the terrain
		Vector3 position = Vector3(GetRandomFloatInRange(terrainExtents.mins.x, terrainExtents.maxs.x), 0.0f, GetRandomFloatInRange(terrainExtents.mins.y, terrainExtents.maxs.y));
		position = GetTerrain()->GetTerrainPosition(position);


		// Get the distance to the nearest enemy(straight line distance)
		float minBE = GetMinDistanceToPositon(position, m_bombEnemies);
		float minBES = GetMinDistanceToPositon(position, m_bombEnemySpawners);
		float min = Min(minBE, minBES);


		// If we are far enough away
		if (min >= m_safeDistance)
		{
			// Get our data and get out
			spawnPosition = position;
			isSafe = true;
		}
	}

	return spawnPosition;
}



float PlayState::GetMinDistanceToPositon(const Vector3& position, const std::vector<Entity*>& entities) const
{
	float minDistance = 1000000.0f; // Probably move scientific method

	for (int i = 0; i < (int)entities.size(); ++i)
	{
		Entity* entity = entities[i];

		float distance = (entity->GetTransform().GetWorldPosition() - position).GetLength();
		if (distance < minDistance)
		{
			minDistance = distance;
		}
	}

	return minDistance;
}



RaycastHit3 PlayState::Raycast(const Ray3& ray, float maxDistance) const
{
	RaycastHit3 hitResult;
	float distance = maxDistance;

	for (int i = 0; i < (int)m_bombEnemies.size(); ++i)
	{
		Entity* bombEnemy = m_bombEnemies[i];

		Vector3 hitPosition;
		float hitDistance;
		bool hit = IntersectRayAndSphere(ray, Sphere(bombEnemy->GetTransform().GetWorldPosition(), bombEnemy->GetCollisionRadius()), hitPosition, hitDistance);
		if (hit && hitDistance < distance)
		{
			distance = hitDistance;

			hitResult.hit = true;
			hitResult.position = hitPosition;
		}
	}

	for (int i = 0; i < (int)m_bombEnemySpawners.size(); ++i)
	{
		Entity* spawner = m_bombEnemySpawners[i];

		Vector3 hitPosition;
		float hitDistance;
		AABB3 box = AABB3::FromCenterHalfDim(spawner->GetTransform().GetWorldPosition(), Vector3(spawner->GetCollisionRadius(), spawner->GetCollisionRadius(), spawner->GetCollisionRadius()));
		//bool hit = IntersectRayAndSphere(ray, Sphere(spawner->GetTransform().GetWorldPosition(), spawner->GetCollisionRadius()), hitPosition);
		bool hit = IntersectRayAndAABB3(ray, box, hitPosition, hitDistance);
		if (hit && hitDistance < distance)
		{
			distance = hitDistance;

			hitResult.hit = true;
			hitResult.position = hitPosition;
		}
	}

	if (!hitResult.hit)
	{
		hitResult = GetTerrain()->Raycast(ray, maxDistance);
	}

	return hitResult;
}
