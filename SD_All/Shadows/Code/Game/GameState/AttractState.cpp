#include "Game/GameState/AttractState.hpp"

#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/Blackboard.hpp"
extern Blackboard g_gameConfigBlackboard;

#include "Engine/Rendering/Camera.hpp"
#include "Engine/Rendering/Material.hpp"
#include "Engine/Rendering/MeshUtils.hpp"
#include "Engine/Rendering/Mesh.hpp"

#include "Game/Menus/Menu.hpp"

#include "Game/GameCommon.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Rendering/Renderer.hpp"
#include "Engine/Commands/DevConsole.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/App.hpp"
extern Window* g_theWindow;
extern Renderer* g_theRenderer;
extern DevConsole* g_theDevConsole;
extern App*	g_theApp;

#include "Game/Game.hpp"



void AttractState::Initialize(Game* owner)
{
	GUARANTEE_OR_DIE(owner != nullptr, "AttractState owner must be valid");


	// Basic State
	SetOwner(owner);
	SetName("AttractState");


	// Camera
	m_camera = new Camera();
	m_camera->SetProjection(Matrix4::MakeOrtho2D(Vector2(0.0f, 0.0f), (Vector2)g_theWindow->GetDimensions()));
	m_camera->SetClearColor(RGBA(0,0,0));
	m_camera->SetColorTarget(g_theRenderer->GetDefaultColorTarget());
	m_camera->SetDepthStencilTarget(g_theRenderer->GetDefaultDepthStencilTarget());


	// Background
	m_backgroundMaterial = Material::FromShader("Default_2D");
	m_backgroundMaterial->SetDiffuseTexture(g_gameConfigBlackboard.GetValue("attractStateBackground", "Data/BuiltIns/DEFAULT_TEXTURE.png"), TEXTURE_SAMPLER_TYPE_POINT);

	m_backgroundQuad = CreateAABB2D(AABB2D(Vector2(0.0f, 0.0f), (Vector2)g_theWindow->GetDimensions()));


	// Menu
	m_menu = new Menu();
	m_menu->Initialize(Vector2(0.0f, 0.0f), true, false);
	m_menu->SetActiveColor(RGBA(0,0,0));
	m_menu->SetHoverColor(RGBA(0,255,0));
	m_menu->AddOption("QUIT", MENU_QUIT);
	m_menu->AddOption("PLAY", MENU_PLAY);
	m_menu->SelectOption("PLAY");
	m_menu->SetFontSize(64.0f);
	m_menu->SetExternalPadding(Vector2(0.0f, 0.0f));

	Vector2 amPadding_Total = Vector2(g_theWindow->GetDimensions()) - m_menu->GetDimensions();
	Vector2 amPadding_Percent = Vector2(0.85f, 0.5f);
	Vector2 amPadding = amPadding_Total.GetHadamard(amPadding_Percent);
	m_menu->SetExternalPadding(amPadding);
}



void AttractState::Destroy()
{
	// Camera
	delete m_camera;
	m_camera = nullptr;

	// Background
	delete m_backgroundMaterial;
	m_backgroundMaterial = nullptr;

	delete m_backgroundQuad;
	m_backgroundQuad = nullptr;

	// Menu
	delete m_menu;
	m_menu = nullptr;
}



void AttractState::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	if (!g_theDevConsole->IsOpen())
	{
		// Process menu input
		m_menu->ProcessInput();
		unsigned int frameSelection = m_menu->GetFrameSelection();
		if( (frameSelection == MENU_QUIT) )
		{
			g_theApp->Quit();
		}
		if( (frameSelection == MENU_PLAY) )
		{
			GetOwner()->StartTransitionToState(GAME_STATE_CHARACTER_SELECT);
			GetOwner()->FinishTransition();
		}

		// Process escape key
		if ( g_theInput->WasKeyJustPressed(InputSystem::KEY_ESCAPE) && !g_theDevConsole->WasJustClosed() )
		{
			g_theApp->Quit();
		}
	}
}



void AttractState::Render2D() const
{
	// Draw the background
	g_theRenderer->BindCamera(m_camera);
	g_theRenderer->BindMaterial(m_backgroundMaterial);
	g_theRenderer->DrawMesh(m_backgroundQuad->GetSubMesh(0));
	
	// Draw the menu
	m_menu->Render();
}
