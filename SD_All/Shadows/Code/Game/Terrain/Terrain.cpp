#include "Game/Terrain/Terrain.hpp"

#include "Engine/Rendering/DebugRender.hpp"
#include "Engine/Core/EngineCommon.h"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix4.hpp"

#include "Engine/Rendering/Renderer.hpp"
#include "Engine/Rendering/MeshBuilder.hpp"
#include "Engine/Rendering/Mesh.hpp"
#include "Engine/Rendering/Material.hpp"
#include "Engine/Rendering/Renderable.hpp"

#include "Engine/Math/Ray3.hpp"
#include "Game/Raycast/RaycastHit3.hpp"



Terrain::Terrain()
{

}



Terrain::~Terrain()
{

}



AABB2D Terrain::GetExtents() const
{
	return m_extents;
}



void Terrain::LoadFromImage(const std::string& filepath, const AABB2D& extents, float minHeight, float maxHeight, int numChunks, float numTextureTiles)
{
	UNUSED(numChunks);

	// Copy state
	m_heightMap = Image(filepath);
	m_extents = extents;
	m_heightRange = FloatRange(minHeight, maxHeight);

	IntVector2 heightMapDimensions = m_heightMap.GetDimensions();
	Vector2 xzPos = extents.mins;
	Vector2 distanceBetweenPoints = extents.GetDimensions();
	distanceBetweenPoints.x /= (float)(heightMapDimensions.x - 1);
	distanceBetweenPoints.y /= (float)(heightMapDimensions.y - 1);
	m_worldDistanceBetweenTexels = distanceBetweenPoints;

	Vector2 texCoords = Vector2::ZEROS;
	Vector2 texCoordsBetweenPoints = Vector2::ONES * numTextureTiles;
	texCoordsBetweenPoints.x /= (float)(heightMapDimensions.x - 1);
	texCoordsBetweenPoints.y /= (float)(heightMapDimensions.y - 1);
	m_texCoordsBetweenTexels = texCoordsBetweenPoints;

	IntVector2 chunkCoverage = heightMapDimensions / numChunks;
	for (int x = 0; x < numChunks; ++x)
	{
		IntVector2 startTexel;
		IntVector2 endTexel;

		// Set up the x value for the start and end texel
		startTexel.x = x * chunkCoverage.x;
		endTexel.x = (x + 1) * chunkCoverage.x;
		
		// If we have an odd mapping of texels to chunks make up for it in the final chunk
		if (x == numChunks - 1 && endTexel.x != heightMapDimensions.x - 1)
		{
			endTexel.x = heightMapDimensions.x - 1;
		}

		for (int y = 0; y < numChunks; ++y)
		{
			// Set up the y value for the start and end texel
			startTexel.y = y * chunkCoverage.y;
			endTexel.y = (y + 1) * chunkCoverage.y;

			// If we have an odd mapping of texels to chunks make up for it in the final chunk
			if (y == numChunks - 1 && endTexel.y != heightMapDimensions.y - 1)
			{
				endTexel.y = heightMapDimensions.y - 1;
			}

			LoadChunk(startTexel, endTexel, Vector2(startTexel).GetHadamard(m_texCoordsBetweenTexels));
		}

	}
	//LoadChunk(IntVector2(0,0), IntVector2(32,32), Vector2::ZEROS);
	//LoadChunk(IntVector2(32,0), IntVector2(64,32), Vector2::ZEROS);
	//LoadChunk(IntVector2(0,32), IntVector2(32,64), Vector2::ZEROS);
	//LoadChunk(IntVector2(32,32), IntVector2(64,64), Vector2::ZEROS);
	//
	//
	//m_renderable = m_renderables[0];

	/*MeshBuilder builder;
	builder.Initialize(Renderer::PRIMITIVE_TRIANGLES, false);
	builder.SetVertexColor(RGBA());
	for (int y = 0; y < heightMapDimensions.y - 1; ++y)
	{
		for (int x = 0; x < heightMapDimensions.x -1; ++x)
		{
			// Generate all necessary vertex data
			IntVector2	texel_LL	 = IntVector2(x, y);
			Vector3		position_LL  = GetPositionForTexel(texel_LL);
			Vector2		texCoords_LL = texCoords;
			Vector4		tangent_LL;
			Vector3		normal_LL	 = GetNormalForTexel(texel_LL, &tangent_LL);
			//DebugDraw_Point(1000.0f, position_LL, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
			//DebugDraw_TextTag(1000.0f, position_LL, 16.0f, RGBA(200,200,200), RGBA(200,200,200), "(%.2f, %.2f)", texCoords_LL.x, texCoords_LL.y);

			IntVector2	texel_LR	 = IntVector2(x, y) + IntVector2(1, 0);
			Vector3		position_LR  = GetPositionForTexel(texel_LR);
			Vector2		texCoords_LR = texCoords_LL + Vector2(texCoordsBetweenPoints.x, 0.0f);
			Vector4		tangent_LR;
			Vector3		normal_LR	 = GetNormalForTexel(texel_LR, &tangent_LR);
			//DebugDraw_Point(1000.0f, position_LR, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
			//DebugDraw_TextTag(1000.0f, position_LR, 16.0f, RGBA(200,200,200), RGBA(200,200,200), "(%.2f, %.2f)", texCoords_LR.x, texCoords_LR.y);

			IntVector2	texel_UL	 = IntVector2(x, y) + IntVector2(0, 1);
			Vector3		position_UL  = GetPositionForTexel(texel_UL);
			Vector2		texCoords_UL = texCoords_LL + Vector2(0.0f, texCoordsBetweenPoints.y);
			Vector4		tangent_UL;
			Vector3		normal_UL	 = GetNormalForTexel(texel_UL, &tangent_UL);
			//DebugDraw_Point(1000.0f, position_UL, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
			//DebugDraw_TextTag(1000.0f, position_UL, 16.0f, RGBA(200,200,200), RGBA(200,200,200), "(%.2f, %.2f)", texCoords_UL.x, texCoords_UL.y);

			IntVector2	texel_UR	 = IntVector2(x, y) + IntVector2(1, 1);
			Vector3		position_UR	 = GetPositionForTexel(texel_UR);
			Vector2		texCoords_UR = texCoords_LL + Vector2(texCoordsBetweenPoints.x, texCoordsBetweenPoints.y);
			Vector4		tangent_UR;
			Vector3		normal_UR	 = GetNormalForTexel(texel_UR, &tangent_UR);
			//DebugDraw_Point(1000.0f, position_UR, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
			//DebugDraw_TextTag(1000.0f, position_UR, 16.0f, RGBA(200,200,200), RGBA(200,200,200), "(%.2f, %.2f)", texCoords_UR.x, texCoords_UR.y);


			// Add triangles to mesh
			// Triangle 1
			builder.SetVertexNormal(normal_LL);
			builder.SetVertexTangent(tangent_LL);
			builder.SetVertexUVs(texCoords_LL);
			builder.PushVertex(position_LL);
			//DebugDraw_LineSegment(1000.0f, position_LL, RGBA(255,0,0), position_LL + normal_LL, RGBA(255,0,0), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);

			builder.SetVertexNormal(normal_LR);
			builder.SetVertexTangent(tangent_LR);
			builder.SetVertexUVs(texCoords_LR);
			builder.PushVertex(position_LR);
			//DebugDraw_LineSegment(1000.0f, position_LR, RGBA(255,0,0), position_LR + normal_LR, RGBA(255,0,0), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);

			builder.SetVertexNormal(normal_UL);
			builder.SetVertexTangent(tangent_UL);
			builder.SetVertexUVs(texCoords_UL);
			builder.PushVertex(position_UL);
			//DebugDraw_LineSegment(1000.0f, position_UL, RGBA(255,0,0), position_UL + normal_UL, RGBA(255,0,0), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);


			// Triangle 2
			builder.SetVertexNormal(normal_LR);
			builder.SetVertexTangent(tangent_LR);
			builder.SetVertexUVs(texCoords_LR);
			builder.PushVertex(position_LR);
			//DebugDraw_LineSegment(1000.0f, position_LR, RGBA(0,0,255), position_LR + normal_LR, RGBA(0,0,255), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);

			builder.SetVertexNormal(normal_UR);
			builder.SetVertexTangent(tangent_UR);
			builder.SetVertexUVs(texCoords_UR);			
			builder.PushVertex(position_UR);
			//DebugDraw_LineSegment(1000.0f, position_UR, RGBA(0,0,255), position_UR + normal_UR, RGBA(0,0,255), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);

			builder.SetVertexNormal(normal_UL);
			builder.SetVertexTangent(tangent_UL);
			builder.SetVertexUVs(texCoords_UL);			
			builder.PushVertex(position_UL);
			//DebugDraw_LineSegment(1000.0f, position_UL, RGBA(0,0,255), position_UL + normal_UL, RGBA(0,0,255), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);


			// Move to the next quad
			texCoords.x += texCoordsBetweenPoints.x;
		}
		texCoords.x = 0.0f;
		texCoords.y += texCoordsBetweenPoints.y;
	}


	// Finalize mesh
	builder.GenerateTangents();
	builder.Finalize();
	m_mesh = builder.CreateMesh(Vertex_3DPCUTBN::GetLayout());
	//m_material = Material::FromShader("Terrain");
	m_material = Material::Get("Terrain");
	m_renderable = new Renderable();
	m_renderable->SetModelMatrix(Matrix4());
	m_renderable->SetMesh(m_mesh);
	m_renderable->SetMaterial(m_material);*/
}



void Terrain::LoadChunk(const IntVector2& startTexel, const IntVector2& endTexel, const Vector2& texCoordsStart)
{
	IntVector2 heightMapDimensions = m_heightMap.GetDimensions();
	Vector2 xzPos = m_extents.mins;
	Vector2 distanceBetweenPoints = m_extents.GetDimensions();
	distanceBetweenPoints.x /= (float)(heightMapDimensions.x - 1);
	distanceBetweenPoints.y /= (float)(heightMapDimensions.y - 1);
	m_worldDistanceBetweenTexels = distanceBetweenPoints;

	Vector2 texCoords = texCoordsStart;

	MeshBuilder builder;
	builder.Initialize(Renderer::PRIMITIVE_TRIANGLES, false);
	builder.SetVertexColor(RGBA());
	for (int y = startTexel.y; y < endTexel.y /*- 1*/; ++y)
	{
		for (int x = startTexel.x; x < endTexel.x /*-1*/; ++x)
		{
			// Generate all necessary vertex data
			IntVector2	texel_LL	 = IntVector2(x, y);
			Vector3		position_LL  = GetPositionForTexel(texel_LL);
			Vector2		texCoords_LL = texCoords;
			Vector4		tangent_LL;
			Vector3		normal_LL	 = GetNormalForTexel(texel_LL, &tangent_LL);
			//DebugDraw_Point(1000.0f, position_LL, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
			//DebugDraw_TextTag(1000.0f, position_LL, 16.0f, RGBA(200,200,200), RGBA(200,200,200), "(%.2f, %.2f)", texCoords_LL.x, texCoords_LL.y);

			IntVector2	texel_LR	 = IntVector2(x, y) + IntVector2(1, 0);
			Vector3		position_LR  = GetPositionForTexel(texel_LR);
			Vector2		texCoords_LR = texCoords_LL + Vector2(m_texCoordsBetweenTexels.x, 0.0f);
			Vector4		tangent_LR;
			Vector3		normal_LR	 = GetNormalForTexel(texel_LR, &tangent_LR);
			//DebugDraw_Point(1000.0f, position_LR, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
			//DebugDraw_TextTag(1000.0f, position_LR, 16.0f, RGBA(200,200,200), RGBA(200,200,200), "(%.2f, %.2f)", texCoords_LR.x, texCoords_LR.y);

			IntVector2	texel_UL	 = IntVector2(x, y) + IntVector2(0, 1);
			Vector3		position_UL  = GetPositionForTexel(texel_UL);
			Vector2		texCoords_UL = texCoords_LL + Vector2(0.0f, m_texCoordsBetweenTexels.y);
			Vector4		tangent_UL;
			Vector3		normal_UL	 = GetNormalForTexel(texel_UL, &tangent_UL);
			//DebugDraw_Point(1000.0f, position_UL, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
			//DebugDraw_TextTag(1000.0f, position_UL, 16.0f, RGBA(200,200,200), RGBA(200,200,200), "(%.2f, %.2f)", texCoords_UL.x, texCoords_UL.y);

			IntVector2	texel_UR	 = IntVector2(x, y) + IntVector2(1, 1);
			Vector3		position_UR	 = GetPositionForTexel(texel_UR);
			Vector2		texCoords_UR = texCoords_LL + Vector2(m_texCoordsBetweenTexels.x, m_texCoordsBetweenTexels.y);
			Vector4		tangent_UR;
			Vector3		normal_UR	 = GetNormalForTexel(texel_UR, &tangent_UR);
			//DebugDraw_Point(1000.0f, position_UR, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
			//DebugDraw_TextTag(1000.0f, position_UR, 16.0f, RGBA(200,200,200), RGBA(200,200,200), "(%.2f, %.2f)", texCoords_UR.x, texCoords_UR.y);


			// Add triangles to mesh
			// Triangle 1
			builder.SetVertexNormal(normal_LL);
			builder.SetVertexTangent(tangent_LL);
			builder.SetVertexUVs(texCoords_LL);
			builder.PushVertex(position_LL);
			//DebugDraw_LineSegment(1000.0f, position_LL, RGBA(255,0,0), position_LL + normal_LL, RGBA(255,0,0), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);

			builder.SetVertexNormal(normal_LR);
			builder.SetVertexTangent(tangent_LR);
			builder.SetVertexUVs(texCoords_LR);
			builder.PushVertex(position_LR);
			//DebugDraw_LineSegment(1000.0f, position_LR, RGBA(255,0,0), position_LR + normal_LR, RGBA(255,0,0), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);

			builder.SetVertexNormal(normal_UL);
			builder.SetVertexTangent(tangent_UL);
			builder.SetVertexUVs(texCoords_UL);
			builder.PushVertex(position_UL);
			//DebugDraw_LineSegment(1000.0f, position_UL, RGBA(255,0,0), position_UL + normal_UL, RGBA(255,0,0), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);


			// Triangle 2
			builder.SetVertexNormal(normal_LR);
			builder.SetVertexTangent(tangent_LR);
			builder.SetVertexUVs(texCoords_LR);
			builder.PushVertex(position_LR);
			//DebugDraw_LineSegment(1000.0f, position_LR, RGBA(0,0,255), position_LR + normal_LR, RGBA(0,0,255), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);

			builder.SetVertexNormal(normal_UR);
			builder.SetVertexTangent(tangent_UR);
			builder.SetVertexUVs(texCoords_UR);			
			builder.PushVertex(position_UR);
			//DebugDraw_LineSegment(1000.0f, position_UR, RGBA(0,0,255), position_UR + normal_UR, RGBA(0,0,255), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);

			builder.SetVertexNormal(normal_UL);
			builder.SetVertexTangent(tangent_UL);
			builder.SetVertexUVs(texCoords_UL);			
			builder.PushVertex(position_UL);
			//DebugDraw_LineSegment(1000.0f, position_UL, RGBA(0,0,255), position_UL + normal_UL, RGBA(0,0,255), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);


			// Move to the next quad
			texCoords.x += m_texCoordsBetweenTexels.x;
		}
		texCoords.x = 0.0f;
		texCoords.y += m_texCoordsBetweenTexels.y;
	}


	// Finalize mesh
	builder.GenerateTangents();
	builder.Finalize();
	m_mesh = builder.CreateMesh(Vertex_3DPCUTBN::GetLayout());

	Renderable* renderable = new Renderable();
	renderable->SetModelMatrix(Matrix4());
	renderable->SetMesh(m_mesh);
	renderable->SetMaterial("Terrain");
	m_renderables.push_back(renderable);
}



Vector3 Terrain::GetTerrainPosition(const Vector3& position) const
{
	Vector3 newPositionWorld = position;
	float newHeight = GetHeight(newPositionWorld);
	newPositionWorld.y = newHeight;
	return newPositionWorld;
}



float Terrain::GetHeight(const Vector3& position) const
{
	Vector2 position2D = Vector2(position.x, position.z);
	float height = GetHeight(position2D);
	return height;
}



float Terrain::GetHeight(const Vector2& position) const
{
	IntVector2 heightMapDimensions = m_heightMap.GetDimensions() - IntVector2(1, 1); // Inclusive Dimensions


	Vector2 normalizedPosition;
	normalizedPosition.x = RangeMapFloat(position.x, m_extents.mins.x, m_extents.maxs.x, 0.0f, 1.0f);
	normalizedPosition.y = RangeMapFloat(position.y, m_extents.mins.y, m_extents.maxs.y, 0.0f, 1.0f);
	//DebugDraw_Log(0.0f, "Normalized Position (%f, %f)", normalizedPosition.x, normalizedPosition.y);

	Vector2 floatTexel = normalizedPosition.GetHadamard(Vector2(heightMapDimensions));
	//DebugDraw_Log(0.0f, "FNP float texel (%f, %f)", floatTexel.x, floatTexel.y);

	// Generate ideal texels
	IntVector2 texel_LL = IntVector2(floatTexel);
	IntVector2 texel_LR = texel_LL + IntVector2(1, 0);
	IntVector2 texel_UL = texel_LL + IntVector2(0, 1);
	IntVector2 texel_UR = texel_LL + IntVector2(1, 1);

	Vector2 normalizedQuadPosition = floatTexel - Vector2(texel_LL);
	//DebugDraw_Log(0.0f, "quad position (%f, %f)", normalizedQuadPosition.x, normalizedQuadPosition.y);

	// Clamp texels to the height map
	texel_LR.x = ClampInt(texel_LR.x, 0, heightMapDimensions.x);
	texel_UL.y = ClampInt(texel_UL.y, 0, heightMapDimensions.y);
	texel_UR.x = ClampInt(texel_UR.x, 0, heightMapDimensions.x);
	texel_UR.y = ClampInt(texel_UR.y, 0, heightMapDimensions.y);
	//DebugDraw_Log(0.0f, "LL (%i, %i)", texel_LL.x, texel_LL.y);
	//DebugDraw_Log(0.0f, "LR (%i, %i)", texel_LR.x, texel_LR.y);
	//DebugDraw_Log(0.0f, "UL (%i, %i)", texel_UL.x, texel_UL.y);
	//DebugDraw_Log(0.0f, "UR (%i, %i)", texel_UR.x, texel_UR.y);
	//DebugDraw_WireSphere(0.0f, GetPositionForTexel(texel_LL), 0.1f, RGBA(), RGBA(), DEBUG_RENDER_IGNORE_DEPTH);
	//DebugDraw_WireSphere(0.0f, GetPositionForTexel(texel_LR), 0.1f, RGBA(), RGBA(), DEBUG_RENDER_IGNORE_DEPTH);
	//DebugDraw_WireSphere(0.0f, GetPositionForTexel(texel_UL), 0.1f, RGBA(), RGBA(), DEBUG_RENDER_IGNORE_DEPTH);
	//DebugDraw_WireSphere(0.0f, GetPositionForTexel(texel_UR), 0.1f, RGBA(), RGBA(), DEBUG_RENDER_IGNORE_DEPTH);

	// Get heights for each texel
	float height_LL = GetMappedHeightForTexel(texel_LL);
	float height_LR = GetMappedHeightForTexel(texel_LR);
	float height_UL = GetMappedHeightForTexel(texel_UL);
	float height_UR = GetMappedHeightForTexel(texel_UR);
	//DebugDraw_Log(0.0f, "height LL (%.2f)", height_LL);
	//DebugDraw_Log(0.0f, "height LR (%.2f)", height_LR);
	//DebugDraw_Log(0.0f, "height UL (%.2f)", height_UL);
	//DebugDraw_Log(0.0f, "height UR (%.2f)", height_UR);

	// Bilinear blending
	// Get the height along each row
	float rowHeight_L = Interpolate(height_LL, height_LR, normalizedQuadPosition.x);
	float rowHeight_U = Interpolate(height_UL, height_UR, normalizedQuadPosition.x);
	//DebugDraw_Log(0.0f, "row height L (%.2f)", rowHeight_L);
	//DebugDraw_Log(0.0f, "row height U (%.2f)", rowHeight_U);
	//DebugDraw_WireSphere(0.0f, Vector3(position.x, rowHeight_L, GetPositionForTexel(texel_LL).z), 0.1f, RGBA(0,0,255), RGBA(0,0,255), DEBUG_RENDER_IGNORE_DEPTH);
	//DebugDraw_WireSphere(0.0f, Vector3(position.x, rowHeight_U, GetPositionForTexel(texel_UL).z), 0.1f, RGBA(255,255,0), RGBA(255,255,0), DEBUG_RENDER_IGNORE_DEPTH);

	// Interpolate between the interpolated heights
	float height = Interpolate(rowHeight_L, rowHeight_U, normalizedQuadPosition.y);
	//DebugDraw_Log(0.0f, "height (%.2f)", height);
	//DebugDraw_WireSphere(0.0f, Vector3(position.x, height, position.y), 0.1f, RGBA(0,255,255), RGBA(0,255,255), DEBUG_RENDER_IGNORE_DEPTH);

	return height;
}



Vector3 Terrain::GetNormal(const Vector3& position) const
{
	Vector2 position2D = Vector2(position.x, position.z);
	Vector3 normal = GetNormal(position2D);
	return normal;
}



Vector3 Terrain::GetNormal(const Vector2& position) const
{
	IntVector2 heightMapDimensions = m_heightMap.GetDimensions() - IntVector2(1, 1); // Inclusive Dimensions


	Vector2 normalizedPosition;
	normalizedPosition.x = RangeMapFloat(position.x, m_extents.mins.x, m_extents.maxs.x, 0.0f, 1.0f);
	normalizedPosition.y = RangeMapFloat(position.y, m_extents.mins.y, m_extents.maxs.y, 0.0f, 1.0f);

	Vector2 floatTexel = normalizedPosition.GetHadamard(Vector2(heightMapDimensions));

	// Generate ideal texels
	IntVector2 texel_LL = IntVector2(floatTexel);
	IntVector2 texel_LR = texel_LL + IntVector2(1, 0);
	IntVector2 texel_UL = texel_LL + IntVector2(0, 1);
	IntVector2 texel_UR = texel_LL + IntVector2(1, 1);

	Vector2 normalizedQuadPosition = floatTexel - Vector2(texel_LL);

	// Clamp texels to the height map
	texel_LR.x = ClampInt(texel_LR.x, 0, heightMapDimensions.x);
	texel_UL.y = ClampInt(texel_UL.y, 0, heightMapDimensions.y);
	texel_UR.x = ClampInt(texel_UR.x, 0, heightMapDimensions.x);
	texel_UR.y = ClampInt(texel_UR.y, 0, heightMapDimensions.y);
	//DebugDraw_WireSphere(0.0f, GetPositionForTexel(texel_LL), 0.1f, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
	//DebugDraw_WireSphere(0.0f, GetPositionForTexel(texel_LR), 0.1f, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
	//DebugDraw_WireSphere(0.0f, GetPositionForTexel(texel_UL), 0.1f, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
	//DebugDraw_WireSphere(0.0f, GetPositionForTexel(texel_UR), 0.1f, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);

	// Get heights for each texel
	Vector3 normal_LL = GetNormalForTexel(texel_LL);
	Vector3 normal_LR = GetNormalForTexel(texel_LR);
	Vector3 normal_UL = GetNormalForTexel(texel_UL);
	Vector3 normal_UR = GetNormalForTexel(texel_UR);

	// Bilinear blending
	// Get the height along each row
	Vector3 rowNormal_L = Interpolate(normal_LL, normal_LR, normalizedQuadPosition.x);
	Vector3 rowNormal_U = Interpolate(normal_UL, normal_UR, normalizedQuadPosition.x);
	//DebugDraw_WireSphere(0.0f, Vector3(position.x, rowHeight_L, GetPositionForTexel(texel_LL).z), 0.1f, RGBA(0,0,255), RGBA(0,0,255), DEBUG_RENDER_USE_DEPTH);
	//DebugDraw_WireSphere(0.0f, Vector3(position.x, rowHeight_U, GetPositionForTexel(texel_UL).z), 0.1f, RGBA(255,255,0), RGBA(255,255,0), DEBUG_RENDER_USE_DEPTH);

	// Interpolate between the interpolated heights
	Vector3 normal = Interpolate(rowNormal_L, rowNormal_U, normalizedQuadPosition.y).GetNormalized();
	float height = GetHeight(position);
	Vector3 position3 = Vector3(position.x, height, position.y);
	//DebugDraw_LineSegment(0.0f, position3, RGBA(0,0,255), position3 + normal, RGBA(0,0,255), RGBA(), RGBA(), DEBUG_RENDER_IGNORE_DEPTH);
	//DebugDraw_WireSphere(0.0f, Vector3(position.x, height, position.y), 0.1f, RGBA(0,255,255), RGBA(0,255,255), DEBUG_RENDER_USE_DEPTH);

	return normal;
}



bool Terrain::IsOnTerrain(const Vector3& position) const
{
	Vector2 position2D = Vector2(position.x, position.z);
	bool isValid = IsOnTerrain(position2D);
	return isValid;
}



bool Terrain::IsOnTerrain(const Vector2& position) const
{
	bool isValid = m_extents.IsPointInside(position);
	return isValid;
}



Vector3 Terrain::GetPositionForTexel(const IntVector2& texel) const
{
	Vector2 origin = m_extents.mins;
	Vector2 xzPosition = origin + m_worldDistanceBetweenTexels.GetHadamard(Vector2(texel));
	float texelHeight = GetMappedHeightForTexel(texel);
	
	Vector3 position = Vector3(xzPosition.x, texelHeight, xzPosition.y);
	return position;
}



float Terrain::GetMappedHeightForTexel(const IntVector2& texel) const
{
	float texelHeight = (float)m_heightMap.GetTexel(texel.x, texel.y).r;
	float height = RangeMapFloat(texelHeight, 0, 255, m_heightRange.min, m_heightRange.max);
	return height;
}



Vector3	Terrain::GetNormalForTexel(const IntVector2& texel, Vector4* out_tangent) const
{
	IntVector2 dim = m_heightMap.GetDimensions();
	IntVector2 texel_up;
	IntVector2 texel_down;
	IntVector2 texel_left;
	IntVector2 texel_right;
	if (texel.x == 0)								// Clamp left
	{
		texel_left = texel;
	}
	else
	{
		texel_left = texel - IntVector2(1, 0);
	}
	if (texel.x >= m_heightMap.GetDimensions().x - 1)	// Clamp right
	{
		texel_right = texel;
	}
	else
	{
		texel_right = texel + IntVector2(1, 0);
	}
	if (texel.y == 0)								// Clamp bottom
	{
		texel_down = texel;
	}
	else
	{
		texel_down = texel - IntVector2(0, 1);
	}
	if (texel.y >= m_heightMap.GetDimensions().y - 1)	// Clamp top
	{
		texel_up = texel;
	}
	else
	{
		texel_up = texel + IntVector2(0, 1);
	}


	Vector3 du = GetPositionForTexel( texel_right ) - GetPositionForTexel( texel_left ); 
	Vector3 tangent = du.GetNormalized(); 

	Vector3 dv = GetPositionForTexel( texel_up ) - GetPositionForTexel( texel_down ); 
	Vector3 bitangent = dv.GetNormalized(); 
	//bitangent = Vector3(0.0f, 0.0f, 1.0f);

	if (out_tangent != nullptr)
	{
		*out_tangent = Vector4(tangent, 1.0f);
	}

	Vector3 normal = CrossProduct( bitangent, tangent ); 
	return normal; 
}



Vector2 Terrain::GetFloatTexelForPosition(const Vector3& position) const
{
	Vector2 pos2 = Vector2(position.x, position.z);
	return GetFloatTexelForPosition(pos2);
}



Vector2 Terrain::GetFloatTexelForPosition(const Vector2& position) const
{
	Vector2 normalizedPosition;
	normalizedPosition.x = RangeMapFloat(position.x, m_extents.mins.x, m_extents.maxs.x, 0.0f, 1.0f);
	normalizedPosition.y = RangeMapFloat(position.y, m_extents.mins.y, m_extents.maxs.y, 0.0f, 1.0f);

	Vector2 floatTexel = normalizedPosition.GetHadamard(Vector2(m_heightMap.GetDimensions()));

	return floatTexel;
}



Vector2 Terrain::GetBeryCoords(const Vector2 position, const Vector2& a, const Vector2& b, const Vector2& c) const
{

	// Compute vectors        
	Vector2 v0 = c - a;
	Vector2 v1 = b - a;
	Vector2 v2 = position - a;

	// Compute dot products
	float dot00 = DotProduct(v0, v0);
	float dot01 = DotProduct(v0, v1);
	float dot02 = DotProduct(v0, v2);
	float dot11 = DotProduct(v1, v1);
	float dot12 = DotProduct(v1, v2);

	// Compute barycentric coordinates
	float invDenom = 1.0f / ((dot00 * dot11) - (dot01 * dot01));
	float u = (dot00 * dot12 - dot01 * dot02) * invDenom;
	float v = (dot11 * dot02 - dot01 * dot12) * invDenom;

	// Check if point is in triangle
	//return (u >= 0) && (v >= 0) && (u + v < 1)
	return Vector2(u, v);
}



Renderable* Terrain::GetRenderable() const
{
	return m_renderable;
}



const std::vector<Renderable*>& Terrain::GetRenderables() const
{
	return m_renderables;
}



RaycastHit3 Terrain::Raycast(const Ray3& ray, float maxDistance) const
{
	//DebugDraw_WireSphere(1000.0f, ray.start, 0.1f, RGBA(0,255,0), RGBA(0,255,0), DEBUG_RENDER_USE_DEPTH);
	//DebugDraw_LineSegment(1000.0f, ray.start, RGBA(), ray.GetPositionAtDistance(maxDistance), RGBA(), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
	//DebugDraw_WireSphere(1000.0f, ray.GetPositionAtDistance(maxDistance), 0.1f, RGBA(255,0,0), RGBA(255,0,0), DEBUG_RENDER_USE_DEPTH);

	float distanceTraveled = 0.0f;
	float stepDistance = 0.5f;
	int	numStepHalves = 0;
	int maxNumStepHalves = 16;

	bool hit = false;

	// Determine how for we should step
	while(distanceTraveled <= maxDistance)
	{
		// we have a point
		if (numStepHalves > maxNumStepHalves)
		{
			hit = true;
			break;
		}


		float testDistance = distanceTraveled + stepDistance;
		
		Vector3 testPosition = ray.GetPositionAtDistance(testDistance);
		//testPosition.x = ClampFloat(testPosition.x, m_extents.mins.x, m_extents.maxs.x);
		//testPosition.y = ClampFloat(testPosition.y, m_heightRange.min, m_heightRange.max);
		//testPosition.z = ClampFloat(testPosition.z, m_extents.mins.y, m_extents.maxs.y);
		if (!m_extents.IsPointInside(testPosition.XZ()))
		{
			distanceTraveled += stepDistance;
			continue;
		}

		float heightAtPosition = GetHeight(testPosition);
		if (testPosition.y < heightAtPosition)
		{
			// We crossed under the terrain
			//DebugDraw_WireSphere(1000.0f, testPosition, 0.01f, RGBA(255,0,0), RGBA(255,0,0), DEBUG_RENDER_USE_DEPTH);

			// halve the stepDistance and start from the last position again
			stepDistance *= 0.5f;
			++numStepHalves;
		}
		else
		{
			//DebugDraw_WireSphere(1000.0f, testPosition, 0.01f, RGBA(255,255,0), RGBA(255,255,0), DEBUG_RENDER_USE_DEPTH);
			// Update for next check
			distanceTraveled += stepDistance;
		}
		
	}

	RaycastHit3 result = RaycastHit3();
	if (hit)
	{
		result.hit = true;
		result.position = ray.GetPositionAtDistance(distanceTraveled);
		result.normal = GetNormal(result.position);
	}


	return result;
}