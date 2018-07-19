#pragma once

#include "Game/Entity/Entity.hpp"

#include <vector>

class Stopwatch;
class BombEnemy;



class BombEnemySpawner : public Entity
{
public:
	BombEnemySpawner(PlayState* owner, unsigned int maxChildren = 10, float spawnTime = 1.0f);
	virtual ~BombEnemySpawner() override;

	virtual void UpdateSelf(float deltaSeconds) override;
	void RemoveDeadChildren();
	void TrySpawn();
	void Spawn();


private:
	std::vector<BombEnemy*> m_children;
	int						m_maxChildren	= 10;
	float					m_spawnTime		= 1.0f;
	Stopwatch*				m_spawnStopwatch = nullptr;
	float					m_spawnDistance = 2.0f;
};