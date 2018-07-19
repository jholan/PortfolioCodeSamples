#pragma once

#include <string>

class Entity;



class HealthPool
{
public:
	HealthPool(Entity* owner, float max, float current = -1.0f);
	~HealthPool();

	float							GetMaxHealth() const;

	float							GetHealth() const;
	void							SetHealth(float amount);
	void							SetHealth(float amount, const std::string& conditionalTags);
	bool							IncrementHealth(float amount);
	bool							IncrementHealth(float amount, const std::string& conditionalTags);
	bool							DecrementHealth(float amount);
	bool							DecrementHealth(float amount, const std::string& conditionalTags);


private:
	Entity*							m_owner;

	float							m_currentHealth = 100.0f;
	float							m_maxHealth		= 100.0f;
};