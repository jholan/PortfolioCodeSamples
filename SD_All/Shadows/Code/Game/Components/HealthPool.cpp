#include "Game/Components/HealthPool.hpp"

#include "Engine/Core/EngineCommon.h"
#include "Engine/Math/MathUtils.hpp"

#include "Game/Entity/Entity.hpp"
#include "Game/Components/Tags.hpp"




HealthPool::HealthPool(Entity* owner, float max, float current)
{
	GUARANTEE_OR_DIE(owner != nullptr, "HealthPool owner cannot be nullptr");
	GUARANTEE_OR_DIE(max > 0.0f, "MAX HEALTH OF 0.0f IS NOT SUPPORTED");

	m_owner = owner;
	m_maxHealth = max;

	// Set the current health, if it wasn't specified OR it was negative OR you start dead set it to max
	m_currentHealth = current;
	if (current <= 0.0f)
	{
		SetHealth(max);
	}
	else
	{
		SetHealth(current);
	}

}



HealthPool::~HealthPool()
{

}



float HealthPool::GetMaxHealth() const
{
	return m_maxHealth;
}



float HealthPool::GetHealth() const
{
	return m_currentHealth;
}



void HealthPool::SetHealth(float amount)
{
	m_currentHealth = amount;
	m_currentHealth = ClampFloat(m_currentHealth, 0.0f, m_maxHealth);

	TODO("Die() in entity self update? entity feature? callback?");
}



void HealthPool::SetHealth(float amount, const std::string& conditionalTags)
{
	if (m_owner->GetTags().HasCSV(conditionalTags))
	{
		SetHealth(amount);
	}
}



bool HealthPool::IncrementHealth(float amount)
{
	GUARANTEE_OR_DIE(amount > 0.0f, "DONT INCREMENT WITH NEGATIVE VALUES, USE DECREMENT");

	// Cache our old health
	float initialHealth = m_currentHealth;


	// Do the heal
	m_currentHealth += amount;
	m_currentHealth = ClampFloat(m_currentHealth, 0.0f, m_maxHealth);


	// Check to see if we actually healed
	bool wasHealed = false;
	if (initialHealth < m_currentHealth)
	{
		wasHealed = true;
	}

	return wasHealed;
}



bool HealthPool::IncrementHealth(float amount, const std::string& conditionalTags)
{
	bool wasHealed = false;

	// If we have the correct tags do the heal
	if (m_owner->GetTags().HasCSV(conditionalTags))
	{
		wasHealed = IncrementHealth(amount);
	}

	return wasHealed;
}



bool HealthPool::DecrementHealth(float amount)
{
	GUARANTEE_OR_DIE(amount > 0.0f, "DONT DECREMENT WITH NEGATIVE VALUES, USE INCREMENT");

	// Cache our old health
	float initialHealth = m_currentHealth;


	// Do the damage
	m_currentHealth -= amount;
	m_currentHealth = ClampFloat(m_currentHealth, 0.0f, m_maxHealth);


	// Check to see if we actually damaged
	bool wasDamaged = false;
	if (initialHealth > m_currentHealth)
	{
		wasDamaged = true;
	}

	return wasDamaged;
}



bool HealthPool::DecrementHealth(float amount, const std::string& conditionalTags)
{
	bool wasDamaged = false;

	// If we have the correct tags do the damage
	if (m_owner->GetTags().HasCSV(conditionalTags))
	{
		wasDamaged = DecrementHealth(amount);
	}

	return wasDamaged;
}