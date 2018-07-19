#pragma once

#include <string>

class Game;



class GameState
{
public:
	// Composition
	GameState()	 {};
	~GameState() {};
	virtual void Initialize(Game* owner);
	virtual void Destroy();

	// Updating
	virtual void Update(float deltaSeconds);
	virtual void Render2D() const;
	virtual void Render3D() const;


	void SetOwner(Game* owner);
	Game* GetOwner() const;


	void SetName(const std::string& name);
	const std::string GetName() const;



private:
	Game*		m_owner = nullptr;
	std::string m_name;
};