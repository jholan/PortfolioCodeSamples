#pragma once
#include "Game/GameState/GameState.hpp"

#include <vector>
#include <string>

#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/Transform.hpp"

class Camera;
class Material;
class Mesh;
class Menu;
class Light;
class Renderable;
class RenderScene;
class ForwardRenderPath;



class DFSState : public GameState
{
public:
	// Composition
	virtual void	Initialize(Game* owner);
	virtual void	Destroy();

	// Updating
	virtual void	Update(float deltaSeconds);
	void			UpdatePauseState();
	void			UpdateFreeCamera(float deltaSeconds);

	// Rendering
	virtual void	Render3D() const;
	virtual void	Render2D() const;
	void			RenderPauseOverlay() const;

	RenderScene*	GetRenderScene() const;
	ForwardRenderPath* GetForwardRenderPass() const;

	// Pause
	bool			IsPaused() const;
	void			Pause();
	void			Unpause();
	void			TogglePause();
	void			SetPauseState(bool state);

	// State Change Input Processing
	void			ProcessStateInput();
	void			ProcessStateInput_Paused();

	Camera*			GetSceneCamera() const;



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
	Vector3					m_sceneCameraStartEulers = Vector3(0.0f, 00.0f, 0.0f);
	Vector3					m_sceneCameraStartPos	 = Vector3(10.0f, 5.0f, -45.0f);


	Vector3					m_theSunLightDirection	= Vector3(1.0f, /*-0.3f*/-1.0f, 1.0f).GetNormalized();



};