#pragma once

#include <string>

#include "Engine/Core/RGBA.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/AABB2D.hpp"
#include "Engine/Rendering/SpriteSheet.hpp"

class GameState;

enum eGameState
{
	GAME_STATE_INVALID,

	GAME_STATE_LOADING,
	GAME_STATE_ATTRACT,
	GAME_STATE_CHARACTER_SELECT,
	GAME_STATE_PLAYING,

	GAME_STATE_COUNT
};
GameState* CreateGameStateFromGameStateEnum(eGameState gameState);



class Game
{
public:
	// Lifetime
	Game()  {};
	~Game() {};

	void Initialize();
	void Destroy();

	// Core
	void Update(float deltaSeconds);
	void Render2D() const;
	void Render3D() const;

	// Developer Mode
	bool InDeveloperMode() const;
	void ToggleDeveloperMode();
	void SetDeveloperMode(bool state);

	// State
	std::string GetCurrentGameStateName() const;
	void		StartTransitionToState(eGameState newGameState);
	void		FinishTransition();

private:
	// Cheats
	bool m_inDeveloperMode;

	// State
	eGameState m_previousState		= GAME_STATE_LOADING;
	eGameState m_currentState		= GAME_STATE_LOADING;
	eGameState m_transitionToState	= GAME_STATE_INVALID;
	GameState* m_currentGameState	= nullptr;

	float m_secondsInCurrentState	= 0.0f;
	float m_secondsInTransition		= 0.0f;

	bool m_isFinishedTransitioning	= false;
};