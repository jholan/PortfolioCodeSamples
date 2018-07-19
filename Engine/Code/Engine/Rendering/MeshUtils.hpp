#pragma once

#include <string>

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/AABB2D.hpp"
#include "Engine/Core/RGBA.hpp"

class SubMesh;
class MeshBuilder;
class Mesh;
class BitmapFont;
class MeshBuilder;

extern const int NUMBER_OF_SIDES_FOR_CIRCLES;


// ------------------------------------------------------------------------------------------------------
// -- 2D
// ------------------------------------------------------------------------------------------------------
Mesh* CreateLine(const Vector2& a, const RGBA& aColor, const Vector2& b, const RGBA& bColor);
Mesh* CreateOutline(const Vector2* points, const RGBA* colors, unsigned int count);
Mesh* CreateCircle(const Vector2& center, float radius, const RGBA& color = RGBA());
Mesh* CreateStippledCircle(const Vector2& center, float radius, const RGBA& color = RGBA());
Mesh* CreateRegularPolygonOutline(const Vector2& center, float radius, int numberOfSides, const RGBA& color = RGBA());
Mesh* CreateAABB2D(const AABB2D& bounds, const RGBA& color = RGBA(), const Vector2& uvsAtMins = Vector2(0.0f, 0.0f), const Vector2& uvsAtMaxs = Vector2(1.0f, 1.0f));



// ------------------------------------------------------------------------------------------------------
// -- 3D
// ------------------------------------------------------------------------------------------------------
Mesh* CreateCube( const Vector3& center, const Vector3& halfDimensions, const RGBA& color = RGBA(), const AABB2D& topUVs = AABB2D::ZERO_TO_ONE, const AABB2D& sideUVs = AABB2D::ZERO_TO_ONE, const AABB2D& bottomUVs = AABB2D::ZERO_TO_ONE);
Mesh* CreateUVSphere( const Vector3& position, float radius, unsigned int wedges, unsigned int slices, const RGBA& color = RGBA() );
Mesh* CreateXYPlane( const Vector3& center, const Vector2& halfDimensions);
Mesh* CreatePlane( const Vector3& center, const Vector2& halfDimensions, const Vector3& right, const Vector3& up);
Mesh* CreateLineSegment( const Vector3& point0, const RGBA& point0Color, const Vector3& point1, const RGBA& point1Color);

Mesh* CreateWireCube( const Vector3& center, const Vector3& halfDimensions, const RGBA& color = RGBA() );
Mesh* CreateWireSphere( const Vector3& position, float radius, unsigned int wedges, unsigned int slices, const RGBA& color = RGBA() );

Mesh* CreateClusterFromOBJ( const std::string& filename );



// ------------------------------------------------------------------------------------------------------
// -- Text
// ------------------------------------------------------------------------------------------------------
void  AddTextToMeshBuilder(MeshBuilder& builder, const Vector3& position, const Vector3& right, const Vector3& up, const RGBA& color, const BitmapFont* font, float fontHeight, float fontAspectScale, const std::string& asciiText, float percentageComplete = 1.0f);
void  AddTextToMeshBuilder(MeshBuilder& builder, const Vector2& position, const RGBA& color, const BitmapFont* font, float fontHeight, float fontAspectScale, const std::string& asciiText, float percentageComplete = 1.0f);