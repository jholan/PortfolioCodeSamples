#include "Game/GameState/GameState.hpp"

#include "Engine/Core/EngineCommon.h"



void GameState::Initialize(Game* owner)
{
	GUARANTEE_OR_DIE(owner != nullptr, "GameState must be initialized with a valid owner");

	SetOwner(owner);
}



void GameState::Destroy()
{

}



void GameState::Update(float deltaSeconds) 
{
	UNUSED(deltaSeconds);
}



void GameState::Render2D() const
{

}


void GameState::Render3D() const
{

}



void GameState::SetOwner(Game* owner)
{
	m_owner = owner;
}



Game* GameState::GetOwner() const
{
	return m_owner;
}



void GameState::SetName(const std::string& name)
{
	m_name = name;
}



const std::string GameState::GetName() const
{
	return m_name;
}
