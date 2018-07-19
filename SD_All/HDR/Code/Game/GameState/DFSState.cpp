#include "Game/GameState/DFSState.hpp"

#include "Engine/Rendering/DebugRender.hpp"
#include "Engine/Profiler/ProfilerVisualizer.hpp"
#include "Engine/Profiler/ProfileScope.hpp"
#include "Engine/Logger/Logger.hpp"

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
#include "Engine/Commands/Command.hpp"
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


// ----DEMO-----------------------------------------------------------
enum eRenderMode
{
	RENDER_MODE_HDR,
	RENDER_MODE_EYE_ADAPTATION
};
eRenderMode renderMode = RENDER_MODE_HDR;

bool renderGeometry = true;
RenderScene* dfsRS = nullptr;

bool addBloom = false;
int  numBloomPasses = 5;
float bloomThreshold = 1.0f;

float split_modeHDR = 0;
float leftTonemap_modeHDR = 1;
float rightTonemap_modeHDR = 0;


// -------------------------------------------------------------------


bool GenerateCubemap(const std::string& filepath, int resolution)
{
	size_t fileExtensionStart = filepath.find_last_of('.');
	if(fileExtensionStart == std::string::npos)
	{
		DebugDraw_Log(100.0f, RGBA(255,0,0), "Could not generate cube map for %s", filepath.c_str());
		return false;
	}

	std::string fileExtension = filepath.substr(fileExtensionStart);
	if (fileExtension != ".hdr")
	{
		DebugDraw_Log(100.0f, RGBA(255,0,0), "Equilateral map %s is not a .hdr file", filepath.c_str());
		return false;
	}

	std::string filepathStart = filepath.substr(0, fileExtensionStart);

	Mesh* unitCube = CreateCube(Vector3::ZEROS, Vector3::ONES);

	Material* generatorMaterial = Material::FromShader("CubeMapGenerator");
	generatorMaterial->SetDiffuseTexture(filepath, Texture::TEXTURE_FORMAT_RGBA16F);

	Camera camera = Camera();
	Texture* renderTarget = Texture::CreateRenderTarget(resolution, resolution, Texture::TEXTURE_FORMAT_RGBA16F);
	Texture* depthTarget = Texture::CreateRenderTarget(resolution, resolution, Texture::TEXTURE_FORMAT_D24S8);

	camera.SetLocalToParentMatrix(Matrix4());
	camera.SetColorTarget(0, renderTarget);
	camera.SetDepthStencilTarget(depthTarget);
	camera.OverrideViewportBaseResolution(true, IntVector2(resolution, resolution));


	// +X side
	camera.SetLocalToParentMatrix(Matrix4::CreateRotationAroundYDegrees(90.0f));
	g_theRenderer->BindCamera(&camera);
	g_theRenderer->ClearDepth();
	g_theRenderer->ClearColor();

	g_theRenderer->BindMaterial(generatorMaterial);
	g_theRenderer->BindModelMatrix(Matrix4());
	g_theRenderer->DrawMesh(unitCube->GetSubMesh(0));

	Image posX = renderTarget->CopyToImage();
	posX.WriteToFile(filepathStart + "_pos_X.hdr", false);



	// -Z side
	camera.SetLocalToParentMatrix(Matrix4::CreateRotationAroundYDegrees(180.0f));
	g_theRenderer->BindCamera(&camera);
	g_theRenderer->ClearDepth();
	g_theRenderer->ClearColor();

	g_theRenderer->BindMaterial(generatorMaterial);
	g_theRenderer->BindModelMatrix(Matrix4());
	g_theRenderer->DrawMesh(unitCube->GetSubMesh(0));

	Image negZ = renderTarget->CopyToImage();
	negZ.WriteToFile(filepathStart + "_neg_Z.hdr", false);


	// -X side
	camera.SetLocalToParentMatrix(Matrix4::CreateRotationAroundYDegrees(270.0f));
	g_theRenderer->BindCamera(&camera);
	g_theRenderer->ClearDepth();
	g_theRenderer->ClearColor();

	g_theRenderer->BindMaterial(generatorMaterial);
	g_theRenderer->BindModelMatrix(Matrix4());
	g_theRenderer->DrawMesh(unitCube->GetSubMesh(0));

	Image negX = renderTarget->CopyToImage();
	negX.WriteToFile(filepathStart + "_neg_X.hdr", false);


	// +Z side
	camera.SetLocalToParentMatrix(Matrix4::CreateRotationAroundYDegrees(0.0f));
	g_theRenderer->BindCamera(&camera);
	g_theRenderer->ClearDepth();
	g_theRenderer->ClearColor();

	g_theRenderer->BindMaterial(generatorMaterial);
	g_theRenderer->BindModelMatrix(Matrix4());
	g_theRenderer->DrawMesh(unitCube->GetSubMesh(0));

	Image posZ = renderTarget->CopyToImage();
	posZ.WriteToFile(filepathStart + "_pos_Z.hdr", false);


	// +Y side
	camera.SetLocalToParentMatrix(Matrix4::CreateRotationAroundXDegrees(-90.0f));
	g_theRenderer->BindCamera(&camera);
	g_theRenderer->ClearDepth();
	g_theRenderer->ClearColor();

	g_theRenderer->BindMaterial(generatorMaterial);
	g_theRenderer->BindModelMatrix(Matrix4());
	g_theRenderer->DrawMesh(unitCube->GetSubMesh(0));

	Image posY = renderTarget->CopyToImage();
	posY.WriteToFile(filepathStart + "_pos_Y.hdr", false);


	// -Y side
	camera.SetLocalToParentMatrix(Matrix4::CreateRotationAroundXDegrees(90.0f));
	g_theRenderer->BindCamera(&camera);
	g_theRenderer->ClearDepth();
	g_theRenderer->ClearColor();

	g_theRenderer->BindMaterial(generatorMaterial);
	g_theRenderer->BindModelMatrix(Matrix4());
	g_theRenderer->DrawMesh(unitCube->GetSubMesh(0));

	Image negY = renderTarget->CopyToImage();
	negY.WriteToFile(filepathStart + "_neg_Y.hdr", false);





	delete renderTarget;
	delete depthTarget;

	delete generatorMaterial;
	
	delete unitCube;

	return true;
};



void GenerateCubemap_Command(Command& cmd)
{
	std::string filepath = cmd.GetNextString();
	int resolution = StringToInt(cmd.GetNextString().c_str());
	bool success = GenerateCubemap(filepath, resolution);

	if(success)
	{
		g_theDevConsole->PrintToLog(RGBA(0,255,0), "Generated Cube Map");
	}
	else
	{
		g_theDevConsole->PrintToLog(RGBA(255,0,0), "Could Not Generate Cube Map");
	}
}



Camera* ghettoCamHook = nullptr;
void DFS_ReloadSkybox_Command(Command& cmd) 
{
	if (ghettoCamHook == nullptr)
	{
		g_theDevConsole->PrintToLog(RGBA(255,0,0), "No Camera");
		return;
	}


	std::string filepath = cmd.GetNextString();
	size_t fileExtensionStart = filepath.find_last_of('.');
	if(fileExtensionStart == std::string::npos)
	{
		g_theDevConsole->PrintToLog(RGBA(255,0,0), Stringf("Specify an extension", filepath.c_str()));
		return;
	}

	std::string fileExtension = filepath.substr(fileExtensionStart);

	std::string basePath = filepath.substr(0, fileExtensionStart);
	Image* skyboxTextures[6] = 
	{
		new Image(basePath + "_neg_X" + fileExtension, false),
		new Image(basePath + "_pos_Z" + fileExtension, false),
		new Image(basePath + "_pos_X" + fileExtension, false),
		new Image(basePath + "_neg_Z" + fileExtension, false),
		new Image(basePath + "_pos_Y" + fileExtension, false),
		new Image(basePath + "_neg_Y" + fileExtension, false)
	};

	TextureCube* skybox = new TextureCube((const Image**)skyboxTextures);
	ghettoCamHook->SetClearSkybox(skybox);


	for (int i = 0; i < 6; ++i)
	{
		delete skyboxTextures[i];
	}
	g_theDevConsole->PrintToLog(RGBA(0,255,0), "Changed skybox");
}


// ---------------------------------------------------------------------------------------------------------------------
void SideTonemap_CycleUp(float& tonemap)
{
	float t = tonemap;
	++t;

	if(t > 3.0f)
	{
		t = 0.0f;
	}

	tonemap = t;
}


void SideTonemap_CycleDown(float& tonemap)
{
	float t = tonemap;
	--t;

	if(t < 0.0f)
	{
		t = 3.0f;
	}

	tonemap = t;
}


bool SideTonemap_Set(const std::string& side, const std::string& name)
{
	std::string n = ToLower(name);

	float* f;
	if(side == "left" || side =="l")
	{
		f = &leftTonemap_modeHDR;
	}
	else if(side == "right" || side =="r")
	{
		f = &rightTonemap_modeHDR;
	}
	else
	{
		return false;
	}


	if (n == "raw")
	{
		*f = 0.0f;
	}
	else if (n == "reinhard")
	{
		*f = 1.0f;
	}
	else if (n == "filmic")
	{
		*f = 2.0f;
	}
	else if (n == "uncharted2" || n == "u2")
	{
		*f = 3.0f;
	}
	else
	{
		return false;
	}

	return true;
}


void DFS_ChangeSideTonemap_Command(Command& cmd) 
{
	std::string name = cmd.GetNextString();
	std::string tonemap = cmd.GetNextString();

	if(name.empty() || tonemap.empty())
	{
		g_theDevConsole->PrintToLog(RGBA(255,0,0), "Fail 2 args required");
	}

	bool success = SideTonemap_Set(name, tonemap);
	if (success)
	{
		g_theDevConsole->PrintToLog(RGBA(0,255,0), Stringf("%s side tone map function is %s", name.c_str(), tonemap.c_str()));
	}
	else
	{
		g_theDevConsole->PrintToLog(RGBA(0,255,0), "fail");
	}
}


void ChangeSplit(float newSplit)
{
	newSplit = ClampFloat(newSplit, 0.0f, 1600.0f);

	split_modeHDR = newSplit;
}


void DFS_ChangeSplit_Command(Command& cmd) 
{
	float split = StringToFloat(cmd.GetNextString().c_str());
	ChangeSplit(split);
}


void DFS_ToggleGeometry_Command(Command& cmd)
{
	renderGeometry = !renderGeometry;
	g_theDevConsole->PrintToLog(RGBA(0,255,0), "Toggled Geometry");
}

void DFS_ToggleBloom_Command(Command& cmd)
{
	addBloom = !addBloom;
	g_theDevConsole->PrintToLog(RGBA(0,255,0), "Toggled Bloom");
}


void DFS_ToggleView_Command(Command& cmd)
{
	std::string mode = cmd.GetNextString();

	if(mode.empty())
	{
		g_theDevConsole->PrintToLog(RGBA(255,0,0), "Requires a view name");
		return;
	}

	mode = ToLower(mode);
	if(mode == "hdr")
	{
		renderMode = RENDER_MODE_HDR;
		g_theDevConsole->PrintToLog(RGBA(0,255,0), "View == HDR");
	}
	else if (mode == "eye")
	{
		renderMode = RENDER_MODE_EYE_ADAPTATION;
		g_theDevConsole->PrintToLog(RGBA(0,255,0), "View == Eye Adaptation");
	}
	else
	{
		g_theDevConsole->PrintToLog(RGBA(255,0,0), "Invalid view");
		return;
	}

	
}


void DFSState::Initialize(Game* owner)
{
	GUARANTEE_OR_DIE(owner != nullptr, "CharacterSelectState owner must be valid");

	RegisterCommand("GenerateCubemap", GenerateCubemap_Command);

	RegisterCommand("DFS_ReloadCubemap", DFS_ReloadSkybox_Command);

	RegisterCommand("DFS_ToggleGeometry", DFS_ToggleGeometry_Command);

	RegisterCommand("DFS_HDR_ChangeSideTonemap", DFS_ChangeSideTonemap_Command);
	RegisterCommand("DFS_HDR_ChangeSplit", DFS_ChangeSplit_Command);

	RegisterCommand("DFS_ChangeView", DFS_ToggleView_Command);

	RegisterCommand("DFS_ToggleBloom", DFS_ToggleBloom_Command);


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


	// Debug Draw
	//DebugDraw_SetGridVisibility(true);
	//DebugDraw_DrawGrid(30, 10, RGBA(50, 50, 50), RGBA(150, 150, 150));


	// Rendering
	m_forwardRenderPath = new ForwardRenderPath();
	m_renderScene = new RenderScene();
	dfsRS = new RenderScene();



	
	// Scene
	m_sceneCamera = new OrbitCamera();
	ghettoCamHook = m_sceneCamera;
	m_sceneCamera->SetColorClearMode(COLOR_CLEAR_MODE_SKYBOX);
	m_sceneCamera->SetClearColor(RGBA(126,87,194));
	m_sceneCamera->SetViewportNormalizedSize(AABB2D(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f))); 
	Image* skyboxTextures[6] = 
	{
		new Image("Data/Images/PaperMill_neg_X.hdr", false),
		new Image("Data/Images/PaperMill_pos_Z.hdr", false),
		new Image("Data/Images/PaperMill_pos_X.hdr", false),
		new Image("Data/Images/PaperMill_neg_Z.hdr", false),
		new Image("Data/Images/PaperMill_pos_Y.hdr", false),
		new Image("Data/Images/PaperMill_neg_Y.hdr", false)

		//new Image("Data/Images/GalaxyCube1.png", false),
		//new Image("Data/Images/GalaxyCube2.png", false),
		//new Image("Data/Images/GalaxyCube3.png", false),
		//new Image("Data/Images/GalaxyCube4.png", false),
		//new Image("Data/Images/GalaxyCube5.png", false),
		//new Image("Data/Images/GalaxyCube6.png", false)
	};

	TextureCube* skybox = new TextureCube((const Image**)skyboxTextures);
	m_sceneCamera->SetClearSkybox(skybox);
	m_sceneCamera->SetDepthClearOptions(true);
	m_sceneCamera->SetColorTarget(0, g_theRenderer->GetDefaultColorTarget());
	//m_sceneCamera->SetColorTarget(1, brightPixelRenderTarget);
	m_sceneCamera->SetDepthStencilTarget(g_theRenderer->GetDefaultDepthStencilTarget());
	float cascadeBreakpoints[3] = {20.0f, 70.0f, 200.0f};
	m_sceneCamera->SetCascadeBreakpoints(cascadeBreakpoints);
	//m_sceneCamera->SetProjectionOrtho(1024.0f / 20.0f / 2.0f, -1000.0f, 1000.0f); // For light projection
	m_sceneCamera->SetProjectionPerspective(18.457f, 0.0f, 0.1f, 1000.0f);
	m_sceneCamera->SetCameraOrder(1);
	m_sceneCameraTransform.SetLocalEulerAngles(m_sceneCameraStartEulers);
	m_sceneCameraTransform.SetLocalPosition(m_sceneCameraStartPos);
	m_renderScene->AddCamera(m_sceneCamera);
	dfsRS->AddCamera(m_sceneCamera);

	for (int i = 0; i < 6; ++i)
	{
		delete skyboxTextures[i];
	}


	// Ambient Light
	//g_theRenderer->SetAmbientLight(RGBA(), 0.2f);



	// The sun
	//m_renderScene->SetSun(m_theSunLightDirection, RGBA(), 0.6f);


	// Fog
	//m_renderScene->SetFogColor(RGBA(100, 100, 100));
	//m_renderScene->SetFogFactors(0.2f, 0.6f);
	//m_renderScene->SetFogDistances(0.0f, 64.0f);


	//DebugDraw_WireSphere(1000.0f, m_sceneCamera->GetPosition(), 0.25f, RGBA(), RGBA(), DEBUG_RENDER_IGNORE_DEPTH);
	//DebugDraw_Frustum(0.0f, m_sceneCamera->GetProjectionForCascade(0), m_sceneCamera->GetLocalToWorldMatrix(), RGBA(0,0,255), RGBA(255,0,255), DEBUG_RENDER_IGNORE_DEPTH);
	//DebugDraw_Frustum(0.0f, m_sceneCamera->GetProjectionForCascade(1), m_sceneCamera->GetLocalToWorldMatrix(), RGBA(0,255,0), RGBA(0,255,0), DEBUG_RENDER_IGNORE_DEPTH);
	//DebugDraw_Frustum(0.0f, m_sceneCamera->GetProjectionForCascade(2), m_sceneCamera->GetLocalToWorldMatrix(), RGBA(255,0,0), RGBA(255,0,0), DEBUG_RENDER_IGNORE_DEPTH);
	//Light* sun = m_renderScene->GetMutableSun();
	//sun->UpdateShadowMaps(m_renderScene, m_sceneCamera);

	//const Texture* archesHDR = Texture::CreateOrGet("Data/Images/Arches.hdr", Texture::TEXTURE_FORMAT_RGBA16F, false);


	


	// -------------------------------------------------------------------------------------------------------------------------------
	// TEST --------------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------------------


	Renderable* floor = new Renderable();
	float hd = 8.0f;
	AABB2D tcs = AABB2D(Vector2(-hd, -hd), Vector2(hd, hd));
	Mesh* floorMesh = CreateCube(Vector3(0.0f, -1.0f, 0.0f), Vector3(200.0f, 1.0f, 200.0f), RGBA(), tcs, tcs, tcs);
	floor->SetMesh(floorMesh);
	floor->SetMaterial("Floor");
	m_renderScene->AddRenderable(floor);
	
	
	Renderable* leftWall = new Renderable();
	hd = 8.0;
	tcs = AABB2D(Vector2(-hd, -hd), Vector2(hd, hd));
	AABB2D tcsShort = AABB2D(Vector2(-hd, -hd) * .005f, Vector2(hd, hd) * .005f);
	Mesh* leftWallMesh = CreateCube(Vector3(0.0f, 5.0f, 0.0f), Vector3(1.0f, 10.0f, 200.0f), RGBA() );
	leftWall->SetMesh(leftWallMesh);
	leftWall->SetMaterial("Floor");
	m_renderScene->AddRenderable(leftWall);
	
	
	Renderable* rightWall = new Renderable();
	hd = 8.0;
	tcs = AABB2D(Vector2(-hd, -hd), Vector2(hd, hd));
	tcsShort = AABB2D(Vector2(-hd, -hd) * .2f, Vector2(hd, hd) * .2f);
	Mesh* rightWallMesh = CreateCube(Vector3(20.0f, 5.0f, 0.0f), Vector3(1.0f, 10.0f, 200.0f), RGBA());
	rightWall->SetMesh(rightWallMesh);
	rightWall->SetMaterial("Floor");
	m_renderScene->AddRenderable(rightWall);
	
	
	
	Renderable* ceiling = new Renderable();
	hd =8.0f;
	tcs = AABB2D(Vector2(-hd, -hd), Vector2(hd, hd));
	Mesh* ceilingMesh = CreateCube(Vector3(0.0f, 16.0f, 0.0f), Vector3(200.0f, 1.0f, 200.0f), RGBA(), tcs, tcs, tcs);
	ceiling->SetMesh(ceilingMesh);
	ceiling->SetMaterial("Floor");
	m_renderScene->AddRenderable(ceiling);
	
	
	
	
	Renderable* backWall = new Renderable();
	hd = 2.0;
	tcs = AABB2D(Vector2(-hd, -hd), Vector2(hd, hd));
	tcsShort = AABB2D(Vector2(-hd, -hd) * .2f, Vector2(hd, hd) * .2f);
	Mesh* backWallMesh = CreateCube(Vector3(0.0f, 5.0f, 50.0f), Vector3(50.0f, 10.0f, 1.0f), RGBA(), tcsShort, tcsShort, tcsShort);
	backWall->SetMesh(backWallMesh);
	backWall->SetMaterial("Floor");
	m_renderScene->AddRenderable(backWall);
	
	
	Renderable* frontWall = new Renderable();
	hd = 2.0;
	tcs = AABB2D(Vector2(-hd, -hd), Vector2(hd, hd));
	tcsShort = AABB2D(Vector2(-hd, -hd) * .2f, Vector2(hd, hd) * .2f);
	Mesh* frontWallMesh = CreateCube(Vector3(0.0f, 5.0f, -200.0f), Vector3(50.0f, 10.0f, 1.0f), RGBA(), tcsShort, tcsShort, tcsShort);
	frontWall->SetMesh(frontWallMesh);
	frontWall->SetMaterial("Floor");
	m_renderScene->AddRenderable(frontWall);
	
	
	
	
	Renderable* box2 = new Renderable();
	Mesh* box2Mesh = CreateUVSphere(Vector3::ZEROS, 0.5f, 100, 100, RGBA());
	box2->SetMesh(box2Mesh);
	box2->SetMaterial("Floor");
	Transform t2;
	t2.SetLocalPosition(Vector3(5.0f, 1.0f, 0.0f));
	box2->SetModelMatrix(t2.GetLocalToWorldMatrix());
	m_renderScene->AddRenderable(box2);
	//
	//
	//
	Light* l1 = new Light();
	l1->InitializePointLight(Vector3(5.0f, 3.0f, 0.0f), RGBA(255,0,0), 1.0f, Vector3(0.0f, 0.0f, 1.0f));
	//l1->InitializeConeLight(Vector3(5.0f, 5.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f).GetNormalized(), 25.0f, 30.0f, RGBA(), .5f, Vector3(1.0f, 0.0f, 0.0f));
	l1->SetShadowCastingState(false);
	m_renderScene->AddLight(l1);
	//DebugDraw_WireSphere(1000.0f, l1->GetPosition(), 0.25f, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
	//DebugDraw_LineSegment(1000.0f, l1->GetPosition(), RGBA(), l1->GetPosition() + (l1->GetDirection() * 25.0f), RGBA(), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
	
	
	Light* l2 = new Light();
	l2->InitializePointLight(Vector3(18.0f, 3.0f, -30.0f), RGBA(0,255,0), 1.0f, Vector3(0.0f, 0.0f, 1.0f));
	//l1->InitializeConeLight(Vector3(5.0f, 5.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f).GetNormalized(), 25.0f, 30.0f, RGBA(), .5f, Vector3(1.0f, 0.0f, 0.0f));
	l2->SetShadowCastingState(false);
	m_renderScene->AddLight(l2);
	//DebugDraw_WireSphere(1000.0f, l1->GetPosition(), 0.25f, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
	//DebugDraw_LineSegment(1000.0f, l1->GetPosition(), RGBA(), l1->GetPosition() + (l1->GetDirection() * 25.0f), RGBA(), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
	
	Light* l3 = new Light();
	l3->InitializePointLight(Vector3(10.0f, 8.0f, 40.0f), RGBA(), 2000.0f, Vector3(0.0f, 0.0f, 1.0f));
	//l3->InitializeConeLight(Vector3(10.0f, 8.0f, 40.0f), Vector3(0.0f, 0.0f, 1.0f).GetNormalized(), 25.0f, 30.0f, RGBA(), 1000000000, Vector3(1.0f, 0.0f, 0.0f));
	l3->SetShadowCastingState(false);
	m_renderScene->AddLight(l3);
	DebugDraw_WireSphere(1000.0f, l3->GetPosition(), 0.25f, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);


	






	Vector3 sd = Vector3(1,1,-0.5).GetNormalized();

	const float ONE_OVER_2_PI = 1.0f / (2.0f * PI);
	const float ONE_OVER_PI = 1.0f / PI;

	float u = atan2(sd.z, sd.x);
	u *= ONE_OVER_2_PI;

	float v = asin(sd.y);
	v *= ONE_OVER_PI;
	v += 0.5f;


	int i = 0;
}



void DFSState::Destroy()
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

	// Rendering
	delete m_forwardRenderPath;
	m_forwardRenderPath = nullptr;

	delete m_renderScene;
	m_renderScene = nullptr;
}







static bool updatePlayer = true;
static bool showDebug = false;
static int  cameraLayerShown = 2;
static int  cameraLayerHidden = 0;
void DFSState::Update(float deltaSeconds)
{
	PROFILE_SCOPE_FUNCTION();

	UNUSED(deltaSeconds);



	// HDR-----------------------------------------------------------------------------------------
	if(g_theInput->IsKeyPressed('Z'))
	{
		ChangeSplit(split_modeHDR - (400.0f * deltaSeconds));
	}
	if(g_theInput->IsKeyPressed('X'))
	{
		ChangeSplit(split_modeHDR + (400.0f * deltaSeconds));
	}

	// Bloom----------------------------------------------------------------------------------------
	if(g_theInput->WasKeyJustPressed('C') && g_theInput->IsKeyPressed(InputSystem::KEY_SHIFT))
	{
		Command c("");
		DFS_ToggleBloom_Command(c);
	}

	// Threshold
	float bloomSpeed = 2.0f;
	if (g_theInput->IsKeyPressed('V') && g_theInput->IsKeyPressed(InputSystem::KEY_SHIFT))
	{
		bloomThreshold -= (bloomSpeed * deltaSeconds);
	}
	if (g_theInput->IsKeyPressed('B') && g_theInput->IsKeyPressed(InputSystem::KEY_SHIFT))
	{
		bloomThreshold += (bloomSpeed * deltaSeconds);
	}
	bloomThreshold = ClampFloat(bloomThreshold, 0.5f, 1000.0f);

	// Passes
	if (g_theInput->WasKeyJustPressed('N') && g_theInput->IsKeyPressed(InputSystem::KEY_SHIFT))
	{
		numBloomPasses -= 1;
	}
	if (g_theInput->WasKeyJustPressed('M') && g_theInput->IsKeyPressed(InputSystem::KEY_SHIFT))
	{
		numBloomPasses += 1;
	}
	numBloomPasses = ClampInt(numBloomPasses, 1, 20);

	if(addBloom)
	{
		DebugDraw_Log(0.0f, "Bloom On  [SHIFT + C]");
		DebugDraw_Log(0.0f, "Bloom Threshold: %.2f [SHIFT + V/B]", bloomThreshold);
		DebugDraw_Log(0.0f, "Bloom Passes: %i [SHIFT + N/M]", numBloomPasses);
	}
	else
	{
		DebugDraw_Log(0.0f, "Bloom Off [SHIFT + C]");
	}



	// Handle pause
	if (IsPaused())
	{
		ProcessStateInput_Paused();
		UpdatePauseState();
	}
	else
	{
		ProcessStateInput();

		// Update Scene
		if (!g_theDevConsole->IsOpen())
		{
			UpdateFreeCamera(deltaSeconds);
			

		}
	}
}



void DFSState::UpdatePauseState()
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



void DFSState::UpdateFreeCamera(float deltaSeconds)
{
	// Rotation
	if(!g_theProfiler->IsCapturingMouse())
	{
		Vector3 eulerAnglesDegrees = m_sceneCameraTransform.GetLocalEulerAngles();
		IntVector2 mouseDelta = g_theInput->GetMouseDelta();
		eulerAnglesDegrees.x += -mouseDelta.y * m_sceneCameraRotSpeed * deltaSeconds;
		eulerAnglesDegrees.y += mouseDelta.x * m_sceneCameraRotSpeed * deltaSeconds;
		eulerAnglesDegrees.x = ClampFloat(eulerAnglesDegrees.x, m_sceneCameraXRange.min, m_sceneCameraXRange.max);
		m_sceneCameraTransform.SetLocalEulerAngles(eulerAnglesDegrees);
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
		m_sceneCameraTransform.TranslateLocal(movementDirection * deltaSeconds* m_sceneCameraMoveSpeed * shiftMutliplier);
	}

	m_sceneCamera->SetLocalToParentMatrix(m_sceneCameraTransform.GetLocalToWorldMatrix());
}


#include "Engine/Rendering/GLFunctions.h"
#include "Engine/Rendering/Sampler.hpp"
Texture* prevLuminanceMap = nullptr;
void DFSState::Render3D() const
{
	PROFILE_SCOPE_FUNCTION();

	if (renderGeometry)
	{
		m_forwardRenderPath->Render(m_renderScene);
		m_forwardRenderPath->ClearAllTemporaryRenderables();
	}
	else
	{
		m_forwardRenderPath->Render(dfsRS);
		m_forwardRenderPath->ClearAllTemporaryRenderables();
	}




	if(addBloom)
	{
		// Gen Bloom Texture
		Texture* brightPixelRenderTarget = Texture::CreateRenderTarget(1600, 900, Texture::TEXTURE_FORMAT_RGBA16F);
		Camera brightPixelCamera;
		brightPixelCamera.SetColorTarget(0, brightPixelRenderTarget);
		Material* brightPixelExtractorMaterial = Material::FromShader("BrightnessExtraction");
		brightPixelExtractorMaterial->SetTexture(0, g_theRenderer->GetDefaultColorTarget(), TEXTURE_SAMPLER_TYPE_LINEAR);
		brightPixelExtractorMaterial->SetProperty("THRESHOLD", bloomThreshold);
		//
		g_theRenderer->BindCamera(&brightPixelCamera);
		g_theRenderer->BindMaterial(brightPixelExtractorMaterial);
		g_theRenderer->DrawMesh(Mesh::GetFSQ()->GetSubMesh(0));
	
		// Blur Bloom Texture
		Texture* blurTarget1 = brightPixelRenderTarget;
		Texture* blurTarget2 = Texture::CreateRenderTarget(1600, 900, Texture::TEXTURE_FORMAT_RGBA16F);
	
		Camera blurCamera;
	
		float isHorizontal = 1.0f;
		Material* blurMaterial = Material::FromShader("Blur_Horizontal");
	
		for (int i = 0; i < (numBloomPasses * 2); ++i)
		{
			Sampler* linearClampSampler = new Sampler();
			linearClampSampler->Initialize(true, false, Sampler::WRAP_MODE_CLAMP_TO_EDGE);
	
			blurMaterial->SetTexture(0, blurTarget1, linearClampSampler);
			blurMaterial->SetProperty("IS_HORIZONTAL", isHorizontal);
	
			blurCamera.SetColorTarget(0, blurTarget2);
	
			g_theRenderer->BindCamera(&blurCamera);
			g_theRenderer->BindMaterial(blurMaterial);
			g_theRenderer->DrawMesh(Mesh::GetFSQ()->GetSubMesh(0));
	
	
			// Flip blur mode and blur target
			if (isHorizontal == 1.0f)
			{
				isHorizontal = 0.0f;
			}
			else
			{
				isHorizontal = 1.0f;
			}
			Texture* tmp = blurTarget2;
			blurTarget2 = blurTarget1;
			blurTarget1 = tmp;
		}


		Material* brightnessSumMaterial = Material::FromShader("BrightnessSum");
		brightnessSumMaterial->SetTexture(2, blurTarget1);
		g_theRenderer->ApplyEffect(brightnessSumMaterial);
		g_theRenderer->FinishEffects();
		delete brightnessSumMaterial;

		delete brightPixelExtractorMaterial;
		delete blurMaterial;
		delete blurTarget1;
		delete blurTarget2;
	}


	

	

	if(renderMode == RENDER_MODE_HDR)
	{
		Material* tonemapper = Material::FromShader("Tonemapper");
		tonemapper->SetProperty("SPLIT", split_modeHDR);
		tonemapper->SetProperty("LEFT_TONEMAP", leftTonemap_modeHDR);
		tonemapper->SetProperty("RIGHT_TONEMAP", rightTonemap_modeHDR);
		g_theRenderer->ApplyEffect(tonemapper);
		g_theRenderer->FinishEffects();
		delete tonemapper;
	}
	else if (renderMode == RENDER_MODE_EYE_ADAPTATION)
	{
		unsigned int dimensions = 1024;
		Texture* currColorTarget = g_theRenderer->GetDefaultColorTarget();
		const Mesh* fsq = Mesh::GetFSQ();
		
		Material* luminanceMaterial = Material::FromShader("FSFX_Desaturation");
		luminanceMaterial->SetProperty("PERCENT_COMPLETE", 1.0f);
		luminanceMaterial->SetDiffuseTexture(currColorTarget, TEXTURE_SAMPLER_TYPE_LINEAR);
		
		if (prevLuminanceMap == nullptr)
		{
			prevLuminanceMap = Texture::CreateRenderTarget(dimensions, dimensions, Texture::TEXTURE_FORMAT_RGB16F, true);
		
			Camera lumMapCamera = Camera();
			lumMapCamera.SetColorTarget(0, prevLuminanceMap);
			lumMapCamera.OverrideViewportBaseResolution(true, IntVector2(dimensions, dimensions));
		
			g_theRenderer->BindCamera(&lumMapCamera);
			g_theRenderer->BindMaterial(luminanceMaterial);
			g_theRenderer->DrawMesh(fsq->GetSubMesh(0));
		
			prevLuminanceMap->GenerateMipmaps();
		}
		
		
		// Generate the current luminance map
		Texture* luminanceColorTarget = Texture::CreateRenderTarget(dimensions, dimensions, Texture::TEXTURE_FORMAT_RGB16F, true);
		
		Camera lumMapCamera = Camera();
		lumMapCamera.SetColorTarget(0, luminanceColorTarget);
		lumMapCamera.OverrideViewportBaseResolution(true, IntVector2(dimensions, dimensions));
		
		g_theRenderer->BindCamera(&lumMapCamera);
		g_theRenderer->BindMaterial(luminanceMaterial);
		g_theRenderer->DrawMesh(fsq->GetSubMesh(0));
		
		luminanceColorTarget->GenerateMipmaps();
		
		
		// Auto Exposure, interpolate exposures
		Material* autoExposureMaterial = Material::FromShader("AutoExposure");
		autoExposureMaterial->SetTexture(0, prevLuminanceMap, TEXTURE_SAMPLER_TYPE_LINEAR_MIPMAPS);
		autoExposureMaterial->SetTexture(1, luminanceColorTarget, TEXTURE_SAMPLER_TYPE_LINEAR_MIPMAPS);
		autoExposureMaterial->SetProperty("EXPOSURE_SPEED", 1.25f);
		autoExposureMaterial->SetProperty("MIP_LEVEL_PREVIOUS", (float)(prevLuminanceMap->GetNumMipmaps() - 1));
		autoExposureMaterial->SetProperty("MIP_LEVEL_CURRENT", (float)(luminanceColorTarget->GetNumMipmaps() - 1));
		
		Camera autoExposureCamera = Camera();
		Texture* autoExposureColorTarget = Texture::CreateRenderTarget(1, 1, Texture::TEXTURE_FORMAT_RGB16F, true);
		autoExposureCamera.SetColorTarget(0, autoExposureColorTarget);
		autoExposureCamera.OverrideViewportBaseResolution(true, IntVector2(1,1));
		
		g_theRenderer->BindCamera(&autoExposureCamera);
		g_theRenderer->BindMaterial(autoExposureMaterial);
		g_theRenderer->DrawMesh(fsq->GetSubMesh(0));
		
		delete prevLuminanceMap;
		prevLuminanceMap = autoExposureColorTarget;


		// Apply Tone mapping
		Material* tonemapper = Material::FromShader("AutoExposureTonemapper");
		tonemapper->SetTexture(2, prevLuminanceMap, TEXTURE_SAMPLER_TYPE_LINEAR_MIPMAPS);
		tonemapper->SetProperty("MIP_LEVEL", (float)(prevLuminanceMap->GetNumMipmaps() - 1));
		tonemapper->SetTexture(3, luminanceColorTarget, TEXTURE_SAMPLER_TYPE_LINEAR_MIPMAPS);
		tonemapper->SetProperty("MIP_LEVEL_LCT", (float)(luminanceColorTarget->GetNumMipmaps() - 1));

		g_theRenderer->ApplyEffect(tonemapper);
		g_theRenderer->FinishEffects();
		delete tonemapper;

		delete luminanceMaterial;
		delete luminanceColorTarget;

	}




	DebugDraw_2DText(0.0f, Vector2(1400.0f, 882.0f), 18.0f, RGBA(), RGBA(), "FPS: %.0f", 1.0f / g_theMasterClock->GetDeltaSeconds());

	GLCheckError(__FILE__, __LINE__);
}



void DFSState::Render2D() const
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



	// Overlays
	if (IsPaused())
	{
		RenderPauseOverlay();
	}
}



void DFSState::RenderPauseOverlay() const
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



RenderScene* DFSState::GetRenderScene() const
{
	return m_renderScene;
}



ForwardRenderPath* DFSState::GetForwardRenderPass() const
{
	return m_forwardRenderPath;
}



//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
bool DFSState::IsPaused() const
{
	return m_isPaused;
}


//------------------------------------------------------------------------
void DFSState::Pause()
{
	m_isPaused = true;
	m_pauseMenu->ShowMenu();
	g_theGameClock->SetPauseState(true);
}


//------------------------------------------------------------------------
void DFSState::Unpause()
{
	m_shouldUnpause = true;
	m_pauseDirection = -1.0f;
}


//------------------------------------------------------------------------
void DFSState::TogglePause()
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
void DFSState::SetPauseState(bool state)
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
void DFSState::ProcessStateInput()
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
void DFSState::ProcessStateInput_Paused()
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
Camera* DFSState::GetSceneCamera() const
{
	return m_sceneCamera;
}
