#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include <cmath>
#include <cassert>
#include <crtdbg.h>

#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/MathUtils.hpp"

#include "Game/App.hpp"

#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Rendering/Renderer.hpp"



//-----------------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int)
{
	UNUSED(applicationInstanceHandle);
	UNUSED(commandLineString);

	g_theApp = new App();
	g_theApp->Initialize();
	g_theApp->RunInternalLoop();
	g_theApp->Destroy();

	return 0;
}
