#pragma once
#include "Game/GameState/GameState.hpp"

class Camera;
class Material;
class Mesh;
class Menu;



class AttractState : public GameState
{
public:
	// Composition
	virtual void Initialize(Game* owner);
	virtual void Destroy();

	// Updating
	virtual void Update(float deltaSeconds);
	virtual void Render2D() const;


private:
	Camera*		m_camera				= nullptr;
	Material*	m_backgroundMaterial	= nullptr;
	Mesh*		m_backgroundQuad		= nullptr;
	Menu*		m_menu					= nullptr;

	const unsigned int	  MENU_PLAY = 1;
	const unsigned int	  MENU_QUIT = 2;
};