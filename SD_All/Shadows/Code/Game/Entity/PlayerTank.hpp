#pragma once
#include "Game/Entity/Entity.hpp"

#include "Engine/Math/FloatRange.hpp"

#include "Engine/Core/Stopwatch.hpp"

class PlayState;
class Mesh;



class PlayerTank : public Entity
{
public:
	PlayerTank(PlayState* owner);
	virtual ~PlayerTank() override;

	virtual void	UpdateSelf(float deltaSeconds) override;
	void			TrySpawnBreadcrumb();
	void			Move(float deltaSeconds);



private:
	// Renderable
	Mesh*			m_mesh = nullptr;

	// Movement
	float			m_movementSpeed = 6.0f;
	float			m_rotationSpeed = 60.0f;
	FloatRange		m_xRotationRange = FloatRange(-30.0f, 30.0f);

	// Breadcrumbs
	Stopwatch		m_breadcrumbStopwatch;
	const float		BREADCRUMB_INTERVAL = 0.4f;
	const float		BREADCRUMB_LIFESPAN = 4.0f;
};