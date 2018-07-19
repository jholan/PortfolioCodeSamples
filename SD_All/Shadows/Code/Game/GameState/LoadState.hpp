#pragma once
#include "Game/GameState/GameState.hpp"

class Material;
class Mesh;
class Camera;



class LoadState : public GameState
{
public:
	// Composition
	virtual void Initialize(Game* owner) override;
	virtual void Destroy()	  override;


	// Updating
	virtual void Update(float deltaSeconds) override;
	virtual void Render2D() const			override;



private:
	// Asset Loading
	void LoadAssets();
	void LoadShaders();
	void LoadMaterials();
	void LoadTextures();
	void LoadModels();
	void LoadAudioGroups();


	int				m_frameNumber			= 0;

	Material*		m_loadScreenMaterial	= nullptr;
	Mesh*			m_backgroundQuad		= nullptr;
	Camera*			m_camera				= nullptr;
};