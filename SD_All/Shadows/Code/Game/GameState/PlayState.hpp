#pragma once
#include "Game/GameState/GameState.hpp"

#include <vector>
#include <string>

#include "Engine/Math/Vector3.hpp"

#include "Engine/Core/Transform.hpp"

#include "Game/Raycast/RaycastHit3.hpp"

#include "Engine/Audio/AudioSystem.hpp"

class Camera;
class Material;
class Mesh;
class Menu;
class Light;
class Renderable;
class RenderScene;
class ForwardRenderPath;
class Terrain;

class Entity;
class PlayerTank;
class BombEnemy;
class BombEnemySpawner;
class Bullet;

class Ray3;



class PlayState : public GameState
{
public:
	// Composition
	virtual void	Initialize(Game* owner);
	virtual void	Destroy();

	// Updating
	virtual void	Update(float deltaSeconds);
	void			UpdatePauseState();
	void			UpdateVictoryState();
	void			UpdateDefeatState();
	void			UpdateCameraForPlayer(float deltaSeconds);
	void			UpdateFreeCamera(float deltaSeconds);
	void			UpdateEnemies(float deltaSeconds);
	void			UpdateBombEnemies(float deltaSeconds);
	void			UpdateBombEnemySpawners(float deltaSeconds);
	void			UpdateBullets(float deltaSeconds);

	// Rendering
	virtual void	Render3D() const;
	virtual void	Render2D() const;
	void			RenderPauseOverlay() const;
	void			RenderVictoryOverlay() const;
	void			RenderDefeatOverlay() const;

	RenderScene*	GetRenderScene() const;
	ForwardRenderPath* GetForwardRenderPass() const;

	// Pause
	bool			IsPaused() const;
	void			Pause();
	void			Unpause();
	void			TogglePause();
	void			SetPauseState(bool state);

	// Victory
	bool			HasWon() const;
	int				GetNumEntitiesWithTags(const std::vector<Entity*>& entities, const std::string& tags) const;

	// Defeat
	bool			HasLost() const;

	// State Change Input Processing
	void			ProcessStateInput();
	void			ProcessStateInput_Paused();
	void			ProcessStateInput_Victory();
	void			ProcessStateInput_Defeat();

	// Entities
	const Terrain*	GetTerrain() const;
	Camera*			GetSceneCamera() const;
	PlayerTank*		GetPlayer() const;
	const std::vector<Entity*>& GetBombEnemies() const; 
	bool			HasEntityWithPointer(void* p) const;
	BombEnemy*		SpawnBombEnemy(const Vector2& xzPosition);
	Bullet*			SpawnBullet(const Vector3& startPosition, const Vector3& direction, float speed, float damage, const std::string& damageConditions, float damageRadius, float lifetimeSeconds, float physicalRadius);

	void			RemoveAllDestroyableEntities();
	void			RemoveDestroyableEntities(std::vector<Entity*>& entities);

	// Collisions
	void			HandleAllCollisions();
	bool			DoSphereSphereCollision(const Vector3& sphere1Position, float sphere1Radius, const Vector3& sphere2Position, float sphere2Radius);

	// Respawn
	Vector3			GetSafeSpawnPositionForPlayer() const;
	float			GetMinDistanceToPositon(const Vector3& position, const std::vector<Entity*>& entities) const;

	// Raycast
	RaycastHit3		Raycast(const Ray3& ray, float maxDistance) const;


private:

	// Pause
	bool					m_isPaused				= false;
	bool					m_shouldUnpause			= false;
	Camera*					m_fullScreenCamera		= nullptr;

	Material*				m_pauseEffect			= nullptr;
	float					m_pauseDirection		= 1.0f;
	float					m_pauseFadeTime			= 0.0f;
	const float				PAUSE_FADE_MAX_TIME		= 0.5f;

	Menu*					m_pauseMenu				= nullptr;
	const unsigned int		PAUSE_MENU_RESUME		= 1;
	const unsigned int		PAUSE_MENU_EXIT			= 2;


	// Victory
	float					m_victoryFadeTime		= 0.0f;
	const float				VICTORY_FADE_MAX_TIME	= 2.0f;
	Material*				m_victoryEffect			= nullptr;

	Menu*					m_victoryMenu			= nullptr;
	const unsigned int		VICTORY_MENU_EXIT		= 1;


	// Defeat
	bool					m_hasLost				= false;
	float					m_defeatFadeDirection   = 1.0f;
	float					m_defeatFadeTime		= 0.0f;
	const float				DEFEAT_FADE_MAX_TIME	= 2.0f;
	Material*				m_defeatEffect			= nullptr;

	Menu*					m_defeatMenu			= nullptr;
	const unsigned int		DEFEAT_MENU_CONTINUE	= 1;
	const unsigned int		DEFEAT_MENU_EXIT		= 2;


	// Game State
	// Rendering
	RenderScene*			m_renderScene			= nullptr;
	ForwardRenderPath*		m_forwardRenderPath		= nullptr;

	// Camera
	Camera*					m_sceneCamera			 = nullptr;
	Transform				m_sceneCameraTransform;	 
	FloatRange				m_sceneCameraXRange		 = FloatRange(-89.0f, 89.0f);
	float					m_sceneCameraRotSpeed	 = 5.0f;
	float					m_sceneCameraMoveSpeed   = 2.5f;
	Vector3					m_sceneCameraStartEulers = Vector3(0.0f, 0.0f, 0.0f);//Vector3(0.0f, -45.0f, 0.0f);
	Vector3					m_sceneCameraStartPos	 = Vector3(0.0f, 5.0f, -5.0f);//Vector3(0.75f, 0.0f, -0.75f);

	Camera*					m_debugCamera			 = nullptr;
	Transform				m_debugCameraTransform;	 
	FloatRange				m_debugCameraXRange		 = FloatRange(-89.0f, 89.0f);
	float					m_debugCameraRotSpeed	 = 5.0f;
	float					m_debugCameraMoveSpeed   = 2.5f;
	Vector3					m_debugCameraStartEulers = Vector3(0.0f, 0.0f, 0.0f);//Vector3(0.0f, -45.0f, 0.0f);
	Vector3					m_debugCameraStartPos	 = Vector3(0.0f, 5.0f, -5.0f);//Vector3(0.75f, 0.0f, -0.75f);



	// Entities
	Terrain*				m_terrain				= nullptr;
	Light*					m_theSun				= nullptr;
	Vector3					m_theSunLightDirection	= Vector3(1.0f, /*-0.3f*/-1.0f, 1.0f).GetNormalized();

	Renderable*				m_water					= nullptr;

	PlayerTank*				m_player				= nullptr;
	std::vector<Entity*>	m_bombEnemies;		
	std::vector<Entity*>	m_bombEnemySpawners;
	std::vector<Entity*>	m_bullets;


	// Respawn 
	float					m_safeDistance			= 10.0f;


	// BGM
	SoundID					m_bgmID;
};