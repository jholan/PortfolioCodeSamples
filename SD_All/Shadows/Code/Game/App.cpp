#include "Game/App.hpp"

#define WIN32_LEAN_AND_MEAN	
#include <windows.h>

#include "Engine/Core/Time.hpp"
#include "Engine/Core/XmlUtilities.hpp"

#include "Engine/Core/EngineCommon.h"
#include "Engine/Core/Blackboard.hpp"

#include "Game/GameCommon.hpp"
#include "Engine/Profiler/Profiler.hpp"
#include "Engine/Profiler/ProfileScope.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Commands/Command.hpp"
#include "Engine/Commands/DevConsole.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Rendering/GLFunctions.h"
#include "Engine/Rendering/Texture.hpp"
#include "Engine/Rendering/Sprite.hpp"
#include "Engine/Rendering/IsoSprite.hpp"
#include "Engine/Rendering/ShaderProgram.h"
#include "Engine/Rendering/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Rendering/DebugRender.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Profiler/ProfilerVisualizer.hpp"

#include "Game/Game.hpp"

extern Window* g_theWindow;


//------------------------------------------------------------------------
void HandleInput( unsigned int msg, size_t wparam, size_t lparam )
{
	UNUSED(lparam);

	switch(msg)
	{
	case WM_KEYDOWN:
	{
		unsigned char asKey = (unsigned char)wparam;
		g_theInput->SetKeyState(asKey, true);
		break;
	}

	case WM_KEYUP:
	{
		unsigned char asKey = (unsigned char)wparam;
		g_theInput->SetKeyState(asKey, false);
		break;
	}

	case WM_MOUSEWHEEL:
	{
		short wheel = GET_WHEEL_DELTA_WPARAM(wparam); 
		float wheelDelta = (float)wheel / (float)WHEEL_DELTA; 
		g_theInput->IncrementWheelDelta(wheelDelta);
		break;
	}

	case WM_MOVE: // Fall through
	case WM_SIZE:
	{
		if (g_theInput->IsMouseLockedToScreen())
		{
			g_theInput->LockMouseToScreen(true);
		}
		break;
	}

	case WM_LBUTTONDOWN: 
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	{
		g_theInput->SetMouseButtonStates(wparam);
		break;
	}

	case WM_ACTIVATE:
	{
		//g_theInput->LockMouseToScreen(false);
		//g_theInput->SetMouseMode(MOUSE_MODE_ABSOLUTE);
		//g_theInput->getmousemode
 		if (wparam == WA_INACTIVE)
 		{
			g_theWindow->SetFocusState(false);
 			ClipCursor(nullptr); // Unlock the mouse
 		}
 		else 
 		{
			g_theWindow->SetFocusState(true);
 			if (g_theInput->IsMouseLockedToScreen() == true)
 			{
 				g_theInput->LockMouseToScreen(true);
 			}
 		}
		break;
	}

	default:
		break;
	}

	

	

}


//------------------------------------------------------------------------
void HandleXOut( unsigned int msg, size_t wparam, size_t lparam )
{
	UNUSED(wparam);
	UNUSED(lparam);

	if (msg == WM_CLOSE)
	{
		g_theApp->Quit();
	}
}


//------------------------------------------------------------------------
bool App::Initialize()
{
	XMLDocument doc;
	doc.LoadFile( "Data/GameConfig.xml" );
	XMLElement* config = doc.FirstChildElement("GameConfig");
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*config);
	doc.Clear();


	g_theMasterClock = new Clock();
	g_theMasterClock->Initialize(nullptr);

	g_theGameClock = new Clock();
	g_theGameClock->Initialize(g_theMasterClock);

	g_theWindow = new Window();
	IntVector2 windowDimensions = g_gameConfigBlackboard.GetValue("windowDimensions", IntVector2(1920, 1080));
	g_theWindow->Initialize(windowDimensions.x, windowDimensions.y);

	g_theRenderer = new Renderer();
	g_theRenderer->Initialize();

	g_theDevConsole = new DevConsole();
	g_theDevConsole->Initialize();

	Profiler_Initialize();

	g_theProfiler = new ProfilerVisualizer();
	g_theProfiler->Initialize();

	g_theInput = new InputSystem();
	g_theInput->Initialize();

	g_theAudio = new AudioSystem();

	DebugDraw_Initialize();

	g_theGame = new Game();
	g_theGame->Initialize();

	g_theWindow->RegisterCallback(&HandleInput);
	g_theWindow->RegisterCallback(&HandleXOut);

	m_isQuitting = false;

	RunCommand("RunScript Data/startup.script");

	return true;
}


//------------------------------------------------------------------------
bool App::Destroy()
{
	m_isQuitting = true;

	g_theWindow->RegisterCallback(&HandleXOut);
	g_theWindow->UnregisterCallback(&HandleInput);

	g_theGame->Destroy();
	delete g_theGame;

	DebugDraw_Destroy();

	delete g_theAudio;

	g_theProfiler->Destroy();

	Profiler_Destroy();

	g_theInput->Destroy();
	delete g_theInput;

	g_theRenderer->Destroy();
	delete g_theRenderer;

	g_theDevConsole->Destroy();
	delete g_theDevConsole;

	g_theWindow->Destroy();
	delete g_theWindow;

	g_theGameClock->Destroy();
	delete g_theGameClock;

	g_theMasterClock->Destroy();
	delete g_theMasterClock;


	return true;
}


//------------------------------------------------------------------------
void App::RunInternalLoop()
{
	// Program main loop; keep running frames until it's time to quit
	while(!IsQuitting())
	{
		RunFrame();
	}
}

//-----------------------------------------------------------------------------------------------
void App::RunFrame()
{
	Profiler_BeginFrame();
	g_theMasterClock->BeginFrame();
	g_theDevConsole->BeginFrame(g_theMasterClock->GetDeltaSeconds());
	g_theRenderer->BeginFrame();
	DebugDraw_BeginFrame();
	g_theInput->BeginFrame();
	g_theAudio->BeginFrame();
	g_theProfiler->BeginFrame();

	if (g_theInput->WasKeyJustPressed('U'))
	{
		g_theGameClock->StepInSeconds( 1.0f / 60.0f, true );
	}

	Update(g_theGameClock->GetDeltaSeconds());
	Render();

	g_theProfiler->EndFrame();
	g_theAudio->EndFrame();
	g_theInput->EndFrame();
	DebugDraw_EndFrame();
	g_theRenderer->EndFrame();
	g_theDevConsole->EndFrame();
	g_theMasterClock->EndFrame();
	Profiler_EndFrame();

}


//------------------------------------------------------------------------
void App::Update(float deltaSeconds)
{
	PROFILE_SCOPE_FUNCTION();

	if (!g_theDevConsole->IsOpen())
	{
		// Dev Console---------------------------------------------------------------------------------------------------------------
		if (g_theInput->WasKeyJustPressed(InputSystem::KEY_TILDE))
		{
			g_theDevConsole->Open();
		}


		// Reload Shaders------------------------------------------------------------------------------------------------------------
		if(g_theInput->WasKeyJustPressed(InputSystem::KEY_F9))
		{
			ShaderProgram::ReloadAllShaderPrograms();
		}


		// Debug Shaders-------------------------------------------------------------------------------------------------------------
		bool reload = false;
		if(g_theInput->WasKeyJustPressed('9'))
		{
			reload = true;

			TODO("Create function to do circular indexes");
			debugShaderProgramIndex -= 1;
			if (debugShaderProgramIndex < 0)
			{
				debugShaderProgramIndex = debugShaderPrograms.size() - 1;
			}
			DebugDraw_Log(2.0f, RGBA(255,0,0), RGBA(255,0,0), "Cycled Shaders Down");
		}
		if (g_theInput->WasKeyJustPressed('0'))
		{
			reload = true;

			TODO("Create function to do circular indexes");
			debugShaderProgramIndex += 1;
			if (debugShaderProgramIndex >= (int)debugShaderPrograms.size())
			{
				debugShaderProgramIndex = 0;
			}
			DebugDraw_Log(2.0f, RGBA(255,0,0), RGBA(255,0,0), "Cycled Shaders Up");
		}
		if (reload)
		{
			reload = false;
			for (size_t i = 0; i < shaderProgramsToModify.size(); ++i)
			{
				ShaderProgram* toModify = ShaderProgram::CreateOrGetMutable(shaderProgramsToModify[i]);

				std::string loadAs = debugShaderPrograms[debugShaderProgramIndex];
				if (loadAs == "self")
				{
					toModify->Reload();
				}
				else
				{
					toModify->ReloadAs(loadAs);
				}
			}
		}
		DebugDraw_2DText(0.0f, Vector2(16.0f, 16.0f), 18.0f, RGBA(), RGBA(), "View [9, 0] - %s", debugShaderProgramDisplayNames[debugShaderProgramIndex]);


		// Screen Shots--------------------------------------------------------------------------------------------------------------
		if (g_theInput->WasKeyJustPressed(InputSystem::KEY_F11))
		{
			Image i = g_theRenderer->GetDefaultColorTarget()->CopyToImage();
			i.WriteToFile("Screenshots/Screenshot.png");
			i.WriteToFile("Screenshots/Screenshot.png", false);
		}
	}


	g_theGame->Update(deltaSeconds);
	DebugDraw_Update();
}


//------------------------------------------------------------------------
void App::Render() const
{
	PROFILE_SCOPE_FUNCTION();

	GLCheckError(__FILE__, __LINE__);
	g_theGame->Render3D();
	
	g_theGame->Render2D();
	
	DebugDraw_Render2D();

	g_theProfiler->Render();

	if (g_theDevConsole->IsOpen())
	{
		g_theDevConsole->Render();
	}
	
}


//------------------------------------------------------------------------
bool App::IsQuitting() const
{
	return m_isQuitting;
}


//------------------------------------------------------------------------
void App::Quit()
{
	m_isQuitting = true;
}
