#include "Game/Game.hpp"

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/MathUtils.hpp"

#include "Game/App.hpp"

#include "Game/GameCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Commands/Command.hpp"
#include "Engine/Commands/DevConsole.hpp"
#include "Engine/Rendering/BitmapFont.hpp"
#include "Engine/Rendering/Camera.hpp"
#include "Engine/Rendering/OrbitCamera.hpp"
#include "Engine/Rendering/Shader.hpp"
#include "Engine/Rendering/Texture.hpp"
#include "Engine/Rendering/ShaderProgram.h"
#include "Engine/Rendering/Mesh.hpp"
#include "Engine/Rendering/MeshUtils.hpp"
#include "Engine/Rendering/Material.hpp"
#include "Engine/Rendering/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"

#include "Game/Menus/Menu.hpp"

#include "Game/GameState/LoadState.hpp"
#include "Game/GameState/AttractState.hpp"
#include "Game/GameState/CharacterSelectState.hpp"
#include "Game/GameState/PlayState.hpp"



//-----------------------------------------------------------------------
GameState* CreateGameStateFromGameStateEnum(eGameState gameState)
{
	GameState* gs = nullptr;

	switch(gameState)
	{
	case GAME_STATE_LOADING:
	{
		gs = new LoadState();
		break;
	}
	case GAME_STATE_ATTRACT:
	{
		gs = new AttractState();
		break;
	}
	case GAME_STATE_CHARACTER_SELECT:
	{
		gs = new CharacterSelectState();
		break;
	}
	case GAME_STATE_PLAYING:
	{
		gs = new PlayState();
		break;
	}
	default:
	{
		TODO("Stringify macro");
		GUARANTEE_OR_DIE(false, Stringf("Specified eGameState does not exist").c_str() );
		break;
	}
	}

	return gs;
}



//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void Quit_Command(Command& cmd)
{
	UNUSED(cmd);

	if (!g_theDevConsole->WasJustClosed())
	{
		g_theApp->Quit();
	}
}


#include "Engine/Core/Image.hpp"
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void Game::Initialize()
{
	m_currentGameState = CreateGameStateFromGameStateEnum(GAME_STATE_LOADING);
	m_currentGameState->Initialize(this);
	g_theInput->SetMouseMode(MOUSE_MODE_RELATIVE);
	g_theInput->SetMouseVisiblity(false);
	g_theInput->LockMouseToScreen(true);
}



//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void Game::Destroy()
{
	m_currentGameState->Destroy();
}



//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void Game::Update(float deltaSeconds)
{
	// Handle state
	m_secondsInCurrentState += g_theMasterClock->GetDeltaSeconds();
	if (m_transitionToState != GAME_STATE_INVALID)
	{
		if (m_isFinishedTransitioning)
		{
			// Cache the next game state so that game states can call StartTransitionToState in Init/Destroy.
			eGameState currentState = m_currentState;
			eGameState nextState = m_transitionToState;

			// Reset state necessary for Init/Destroy transitions
			m_isFinishedTransitioning = false;
			m_transitionToState = GAME_STATE_INVALID;

			// Switch states
			m_currentGameState->Destroy();
			m_currentGameState = CreateGameStateFromGameStateEnum(nextState);
			m_currentGameState->Initialize(this);

			// Update states so we know what our last state was
			// For transitions
			m_previousState = currentState;
			m_currentState = nextState;
			
			// Reset remaining state
			m_secondsInCurrentState = 0.0f;
			m_secondsInTransition = 0.0f;
		}
		else
		{
			m_secondsInTransition += g_theMasterClock->GetDeltaSeconds();
		}
	}


	m_currentGameState->Update(deltaSeconds);
}



void Game::Render2D()	const
{
	m_currentGameState->Render2D();
}



void Game::Render3D()	const
{
	m_currentGameState->Render3D();
}



//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
bool Game::InDeveloperMode() const
{
	return m_inDeveloperMode;
}


//------------------------------------------------------------------------
void Game::ToggleDeveloperMode()
{
	m_inDeveloperMode = !m_inDeveloperMode;
}


//------------------------------------------------------------------------
void Game::SetDeveloperMode(bool state)
{
	m_inDeveloperMode = state;
}



//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
std::string Game::GetCurrentGameStateName() const
{
	GUARANTEE_OR_DIE(m_currentGameState != nullptr, "Cannot get the name of a null GameState");

	return m_currentGameState->GetName();
}


//------------------------------------------------------------------------
void Game::StartTransitionToState(eGameState newGameState)
{
	if (m_transitionToState != newGameState)
	{
		m_transitionToState = newGameState;
		m_secondsInTransition = 0.0f;
	}
}



void Game::FinishTransition()
{
	m_isFinishedTransitioning = true;
}