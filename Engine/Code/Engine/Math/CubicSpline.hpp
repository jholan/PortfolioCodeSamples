#pragma once
#include <vector>
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/MathUtils.hpp"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Standalone curve utility functions
//
// Note: Quadratic Bezier functions (only) specify an actual midpoint, not a Bezier guide point
/////////////////////////////////////////////////////////////////////////////////////////////////

template< typename T >
T EvaluateQuadraticBezier( const T& startPos, const T& actualCurveMidPos, const T& endPos, float t );

template< typename T >
T EvaluateCubicBezier( const T& startPos, const T& guidePos1, const T& guidePos2, const T& endPos, float t );

template< typename T >
T EvaluateCubicHermite( const T& startPos, const T& startVel, const T& endPos, const T& endVel, float t );


/////////////////////////////////////////////////////////////////////////////////////////////////
// CubicSpline2D
// 
// Cubic Hermite/Bezier spline of Vector2 positions / velocities
/////////////////////////////////////////////////////////////////////////////////////////////////
class CubicSpline2D
{
public:
	CubicSpline2D() {}
	explicit CubicSpline2D( const Vector2* positionsArray, int numPoints, const Vector2* velocitiesArray=nullptr );
	~CubicSpline2D() {}

	// Mutators
	void		AppendPoint( const Vector2& position, const Vector2& velocity = Vector2(0.0f, 0.0f) );
	void		AppendPoints( const Vector2* positionsArray, int numPoints, const Vector2* velocitiesArray=nullptr );
	void		InsertPoint( int insertBeforeIndex, const Vector2& position, const Vector2& velocity = Vector2(0.0f, 0.0f) );
	void		RemovePoint( int pointIndex );
	void		RemoveAllPoints();
	void		SetPoint( int pointIndex, const Vector2& newPosition, const Vector2& newVelocity );
	void		SetPosition( int pointIndex, const Vector2& newPosition );
	void		SetVelocity( int pointIndex, const Vector2& newVelocity );
	void		SetCardinalVelocities( float tension=0.f, const Vector2& startVelocity = Vector2(0.0f, 0.0f), const Vector2& endVelocity = Vector2(0.0f, 0.0f) );

	// Accessors
	int				GetNumPoints() const { return (int) m_positions.size(); }
	const Vector2	GetPosition( int pointIndex ) const;
	const Vector2	GetVelocity( int pointIndex ) const;
	int				GetPositions( std::vector<Vector2>& out_positions ) const;
	int				GetVelocities( std::vector<Vector2>& out_velocities ) const;
	Vector2			EvaluateAtCumulativeParametric( float t ) const;
	Vector2			EvaluateAtNormalizedParametric( float t ) const;

protected:
	std::vector<Vector2>	m_positions;
	std::vector<Vector2>	m_velocities;
};



template< typename T >
T EvaluateQuadraticBezier(const T& startPos, const T& actualCurveMidPos, const T& endPos, float t)
{
	T halfwayPoint = Interpolate(startPos, endPos, 0.5f);
	T guidePos = Interpolate(halfwayPoint, actualCurveMidPos, 2.0f);

	T ab = Interpolate(startPos, guidePos, t);
	T bc = Interpolate(guidePos, endPos, t);
	T curvePoint = Interpolate(ab, bc, t);
	
	return curvePoint;
}



template< typename T >
T EvaluateCubicBezier(const T& startPos, const T& guidePos1, const T& guidePos2, const T& endPos, float t)
{
	T ab = Interpolate(startPos, guidePos1, t);
	T bc = Interpolate(guidePos1, guidePos2, t);
	T cd = Interpolate(guidePos2, endPos, t);

	T abc = Interpolate(ab, bc, t);
	T bcd = Interpolate(bc, cd, t);

	T curvePoint = Interpolate(abc, bcd, t);

	return curvePoint;
}



template< typename T >
T EvaluateCubicHermite(const T& startPos, const T& startVel, const T& endPos, const T& endVel, float t)
{
	T guidePos1 = startPos + (startVel / 3.0f);
	T guidePos2 = endPos - (endVel / 3.0f);
	T curvePoint = EvaluateCubicBezier(startPos, guidePos1, guidePos2, endPos, t);

	return curvePoint;
}

