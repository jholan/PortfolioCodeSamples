#pragma once
#include "Engine/Math/AABB2D.hpp"


#define UNUSED(x) (void)(x);

extern const AABB2D DRAW_WINDOW;
extern const float  CLIENT_ASPECT;

class Clock;
extern Clock* g_theMasterClock;

class Clock;
extern Clock* g_theGameClock;

class App;
extern App* g_theApp;

class Window;
extern Window* g_theWindow;

class DevConsole;
extern DevConsole* g_theDevConsole;

class Renderer;
extern Renderer* g_theRenderer;

class InputSystem;
extern InputSystem* g_theInput;

class AudioSystem;
extern AudioSystem* g_theAudio;

class Game;
extern Game* g_theGame;

class ProfilerVisualizer;
extern ProfilerVisualizer* g_theProfiler;



// Game math utilities
extern const float COS_45_DEGREES;
extern const float COS_135_DEGREES;

// Teams
extern const int TEAM_ALLY;
extern const int TEAM_ENEMY;
extern const int TEAM_SPELL;

// Debug
extern bool  g_visualizeSpells;