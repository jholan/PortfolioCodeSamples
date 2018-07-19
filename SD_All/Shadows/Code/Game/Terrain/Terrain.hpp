#pragma once

#include <string>

#include "Engine/Math/AABB2D.hpp"
#include "Engine/Math/FloatRange.hpp"

#include "Engine/Core/Image.hpp"

class Renderable;
class Mesh;
class Material;
class Ray3;
class RaycastHit3;



class Terrain
{
public:
	Terrain();
	~Terrain();

	AABB2D GetExtents() const;

	void LoadFromImage(const std::string& filepath, const AABB2D& extents, float minHeight, float maxHeight, int numChunks, float numTextureTiles = 1.0f);
	void LoadChunk(const IntVector2& startDim, const IntVector2& endDim, const Vector2& texCoordsStart);

	Vector3 GetTerrainPosition(const Vector3& position) const; // Corrents the height for the given position

	float GetHeight(const Vector3& position) const;
	float GetHeight(const Vector2& position) const;
	Vector3 GetNormal(const Vector3& position) const;
	Vector3 GetNormal(const Vector2& position) const;
	bool  IsOnTerrain(const Vector3& position) const;
	bool  IsOnTerrain(const Vector2& position) const;

	Vector3 GetPositionForTexel(const IntVector2& texel) const;
	float	GetMappedHeightForTexel(const IntVector2& texel) const;
	Vector3	GetNormalForTexel(const IntVector2& texel, Vector4* out_tangent = nullptr) const;

	Vector2 GetFloatTexelForPosition(const Vector2& position) const;
	Vector2 GetFloatTexelForPosition(const Vector3& position) const;

	Vector2 GetBeryCoords(const Vector2 position, const Vector2& a, const Vector2& b, const Vector2& c) const;

	Renderable* GetRenderable() const;
	const std::vector<Renderable*>& GetRenderables() const;

	RaycastHit3 Raycast(const Ray3& ray, float maxDistance) const;

private:
	AABB2D		m_extents;
	FloatRange	m_heightRange;
	Image		m_heightMap;

	Vector2		m_worldDistanceBetweenTexels;
	Vector2		m_texCoordsBetweenTexels;

	Renderable*	m_renderable;
	Mesh*		m_mesh;
	Material*	m_material;

	std::vector<Renderable*> m_renderables;
};