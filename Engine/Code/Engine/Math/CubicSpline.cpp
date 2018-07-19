#include "Engine/Math/CubicSpline.hpp"

#include "Engine/Math/MathUtils.hpp"



CubicSpline2D::CubicSpline2D( const Vector2* positionsArray, int numPoints, const Vector2* velocitiesArray )
{
	m_positions.reserve(numPoints);
	m_velocities.reserve(numPoints);

	AppendPoints(positionsArray, numPoints, velocitiesArray);
}



// Mutators
void CubicSpline2D::AppendPoint( const Vector2& position, const Vector2& velocity)
{
	m_positions.push_back(position);
	m_velocities.push_back(velocity);
}



void CubicSpline2D::AppendPoints( const Vector2* positionsArray, int numPoints, const Vector2* velocitiesArray)
{
	for (int i = 0; i < numPoints; ++i)
	{
		m_positions.push_back(positionsArray[i]);
	}

	if (velocitiesArray != nullptr)
	{
		for (int i = 0; i < numPoints; ++i)
		{
			m_velocities.push_back(velocitiesArray[i]);
		}
	}
	else
	{
		// Use Catmull-Rom to generate velocities.
		for (int i = 0; i < numPoints; ++i)
		{
			Vector2 ZeroZero = Vector2(0.0f, 0.0f);
			m_velocities.push_back(ZeroZero);
		}
		SetCardinalVelocities();
	}
}



void CubicSpline2D::InsertPoint( int insertBeforeIndex, const Vector2& position, const Vector2& velocity)
{
	auto posIter = m_positions.begin() + insertBeforeIndex;
	m_positions.insert(posIter, position);

	auto velIter = m_velocities.begin() + insertBeforeIndex;
	m_velocities.insert(velIter, velocity);
}



void CubicSpline2D::RemovePoint( int pointIndex )
{
	auto posIter = m_positions.begin() + pointIndex;
	m_positions.erase(posIter);

	auto velIter = m_velocities.begin() + pointIndex;
	m_velocities.erase(velIter);
}



void CubicSpline2D::RemoveAllPoints()
{
	m_positions.clear();
	m_velocities.clear();
}



void CubicSpline2D::SetPoint( int pointIndex, const Vector2& newPosition, const Vector2& newVelocity )
{
	m_positions[pointIndex] = newPosition;
	m_velocities[pointIndex] = newVelocity;
}



void CubicSpline2D::SetPosition( int pointIndex, const Vector2& newPosition )
{
	m_positions[pointIndex] = newPosition;
}



void CubicSpline2D::SetVelocity( int pointIndex, const Vector2& newVelocity )
{
	m_velocities[pointIndex] = newVelocity;
}



void CubicSpline2D::SetCardinalVelocities( float tension, const Vector2& startVelocity, const Vector2& endVelocity)
{
	if (m_velocities.empty()) { return; } // Short Circuit
	
	int size = (int)m_velocities.size();
	if (size < 2)
	{
		m_velocities[0] = startVelocity;
	}
	else
	{
		m_velocities[0] = startVelocity;
		m_velocities[size - 1] = endVelocity;

		float velocityScale = 1.0f - tension;
		for (int i = 1; i < (size - 1); ++i)
		{
			Vector2 velocityIntoPosition = m_positions[i] - m_positions[i - 1];
			Vector2 velocityOutOfPosition = m_positions[i + 1] - m_positions[i];
			Vector2 averagedVelocityAtPosition = (velocityIntoPosition + velocityOutOfPosition) / 2.0f;
			m_velocities[i] = velocityScale * averagedVelocityAtPosition;
		}
	}
}



// Accessors
const Vector2 CubicSpline2D::GetPosition( int pointIndex ) const
{
	return m_positions[pointIndex];
}



const Vector2 CubicSpline2D::GetVelocity( int pointIndex ) const
{
	return m_velocities[pointIndex];
}



int	CubicSpline2D::GetPositions( std::vector<Vector2>& out_positions ) const
{
	out_positions = m_positions;
	return (int)m_positions.size();
}



int CubicSpline2D::GetVelocities( std::vector<Vector2>& out_velocities ) const
{
	out_velocities = m_velocities;
	return (int)m_velocities.size();
}



Vector2	CubicSpline2D::EvaluateAtCumulativeParametric( float t ) const
{
	int numLineSegments = (int)m_positions.size() - 1;
	int lineSegment = (int)t;
	float t_inLineSegment = t - (float)lineSegment;

	Vector2 curvePoint = m_positions[0];
	if (t < 0.0f)
	{
		curvePoint = m_positions[0];
	}
	else if (t > numLineSegments + 1.0f)
	{
		curvePoint = m_positions[m_positions.size() - 1];
	}
	else
	{
		curvePoint = EvaluateCubicHermite(m_positions[lineSegment], m_velocities[lineSegment], m_positions[lineSegment + 1], m_velocities[lineSegment + 1], t_inLineSegment);
	}

	return curvePoint;
}



Vector2 CubicSpline2D::EvaluateAtNormalizedParametric( float t ) const
{
	int numLineSegments = (int)m_positions.size() - 1;
	float t_cumulative = numLineSegments * t;

	Vector2 curvePoint = EvaluateAtCumulativeParametric(t_cumulative);
	return curvePoint;
}
