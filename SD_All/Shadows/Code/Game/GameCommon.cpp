#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"


const AABB2D DRAW_WINDOW = AABB2D(Vector2(0.0f, 0.0f), Vector2(1000.0f, 1000.0f));
const float  CLIENT_ASPECT = 16.0f / 9.0f;

Clock* g_theMasterClock = nullptr;

Clock* g_theGameClock = nullptr;

App* g_theApp = nullptr;

Window* g_theWindow = nullptr;

DevConsole* g_theDevConsole = nullptr;

Renderer* g_theRenderer = nullptr;

InputSystem* g_theInput = nullptr;

AudioSystem* g_theAudio = nullptr;

Game* g_theGame = nullptr;

ProfilerVisualizer* g_theProfiler = nullptr;


// Game math utils
const float COS_45_DEGREES = CosDegrees(45.0f);
const float COS_135_DEGREES = CosDegrees(135.0f);

// Teams
const int TEAM_ALLY  = 1;
const int TEAM_ENEMY = 2;
const int TEAM_SPELL = 3;


// Debug
bool g_visualizeSpells = false;