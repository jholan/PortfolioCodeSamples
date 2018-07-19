#include "Engine/Rendering/MeshUtils.hpp"

#include <fstream>
#include <iostream>

#include "Engine/Core/EngineCommon.h"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"

#include "Engine/Rendering/Mesh.hpp"
#include "Engine/Rendering/MeshBuilder.hpp"
#include "Engine/Rendering/Renderer.hpp"



const int NUMBER_OF_SIDES_FOR_CIRCLES = 25;



Mesh* CreateLine(const Vector2& a, const RGBA& aColor, const Vector2& b, const RGBA& bColor)
{
	Mesh* line = nullptr;
	MeshBuilder builder;
	builder.Initialize(Renderer::PRIMITIVE_LINES, false);
	builder.SetVertexUVs(0.0f, 0.0f);

	builder.SetVertexColor(aColor);
	builder.PushVertex(a);

	builder.SetVertexColor(bColor);
	builder.PushVertex(b);

	builder.Finalize();
	line = builder.CreateMesh(Vertex_3DPCU::GetLayout());
	return line;
}



Mesh* CreateOutline(const Vector2* points, const RGBA* colors, unsigned int count)
{
	Mesh* outline = nullptr;
	MeshBuilder builder;
	builder.Initialize(Renderer::PRIMITIVE_LINES, false);
	builder.SetVertexUVs(0.0f, 0.0f);

	for (int i = 0; i+1 < (int)count; ++i)
	{
		builder.SetVertexColor(colors[i]);
		builder.PushVertex(points[i]);

		builder.SetVertexColor(colors[i + 1]);
		builder.PushVertex(points[i + 1]);
	}

	builder.Finalize();
	outline = builder.CreateMesh(Vertex_3DPCU::GetLayout());
	return outline;
}



Mesh* CreateCircle(const Vector2& center, float radius, const RGBA& color)
{
	return CreateRegularPolygonOutline(center, radius, NUMBER_OF_SIDES_FOR_CIRCLES, color);
}



Mesh* CreateStippledCircle(const Vector2& center, float radius, const RGBA& color)
{
	int numVerts = NUMBER_OF_SIDES_FOR_CIRCLES; // draw half the number of sides but 2 verts per side == num sides
	if (numVerts % 2 != 0) { ++numVerts; }		// Handle odd number of sides
	float degreesPerLineSegment = 360.0f / (float)NUMBER_OF_SIDES_FOR_CIRCLES;

	Mesh* circle = nullptr;
	MeshBuilder builder;
	builder.Initialize(Renderer::PRIMITIVE_LINES, false);
	builder.SetVertexColor(color);
	builder.SetVertexUVs(0.0f, 0.0f);

	for(int i = 0; i < numVerts; i += 2)
	{
		// Point 1
		float point1Angle = degreesPerLineSegment * i;
		Vector2 point1(center.x + (CosDegrees(point1Angle) * radius), center.y + (SinDegrees(point1Angle) * radius));
		builder.PushVertex(point1);


		// Point 2
		float point2Angle = degreesPerLineSegment * (i + 1);
		Vector2 point2(center.x + (CosDegrees(point2Angle) * radius), center.y + (SinDegrees(point2Angle) * radius));
		builder.PushVertex(point2);
	}

	builder.Finalize();
	circle = builder.CreateMesh(Vertex_3DPCU::GetLayout());
	return circle;
}



Mesh* CreateRegularPolygonOutline(const Vector2& center, float radius, int numberOfSides, const RGBA& color)
{
	int	numVerts = numberOfSides + 1;
	float degreesPerLineSegment = 360.0f / (float)numberOfSides;
	//Vector2* positions = new Vector2[numVerts];

	Mesh* polygon = nullptr;
	MeshBuilder builder;
	builder.Initialize(Renderer::PRIMITIVE_LINES, false);
	builder.SetVertexColor(color);
	builder.SetVertexUVs(0.0f, 0.0f);

	for (int i = 0; i+1 < numVerts; ++i)
	{
		// Point i
		float pointAngle_i = degreesPerLineSegment * i;
		Vector2 point_i = Vector2(center.x + (CosDegrees(pointAngle_i) * radius), center.y + (SinDegrees(pointAngle_i) * radius));
		builder.PushVertex(point_i);

		// Point i + 1
		float pointAngle_ipo = degreesPerLineSegment * (i + 1);
		Vector2 point_ipo = Vector2(center.x + (CosDegrees(pointAngle_ipo) * radius), center.y + (SinDegrees(pointAngle_ipo) * radius));
		builder.PushVertex(point_ipo);
	}

	builder.Finalize();
	polygon = builder.CreateMesh(Vertex_3DPCU::GetLayout());
	return polygon;
}



Mesh* CreateAABB2D(const AABB2D& bounds, const RGBA& color, const Vector2& uvsAtMins, const Vector2& uvsAtMaxs)
{
	Vector2 quadPoints[4];
	quadPoints[0] = Vector2(bounds.mins.x, bounds.mins.y);
	quadPoints[1] = Vector2(bounds.maxs.x, bounds.mins.y);
	quadPoints[2] = Vector2(bounds.maxs.x, bounds.maxs.y);
	quadPoints[3] = Vector2(bounds.mins.x, bounds.maxs.y);

	Vector2 quadUVs[4];
	quadUVs[0] = Vector2(uvsAtMins.x, uvsAtMins.y);
	quadUVs[1] = Vector2(uvsAtMaxs.x, uvsAtMins.y);
	quadUVs[2] = Vector2(uvsAtMaxs.x, uvsAtMaxs.y);
	quadUVs[3] = Vector2(uvsAtMins.x, uvsAtMaxs.y);


	Mesh* quad = nullptr;
	MeshBuilder builder;
	builder.Initialize(Renderer::PRIMITIVE_TRIANGLES, true);
	builder.SetVertexColor(color);

	builder.SetVertexUVs(quadUVs[0]);
	unsigned int startIndex = builder.PushVertex(quadPoints[0]);

	builder.SetVertexUVs(quadUVs[1]);
	builder.PushVertex(quadPoints[1]);

	builder.SetVertexUVs(quadUVs[2]);
	builder.PushVertex(quadPoints[2]);

	builder.SetVertexUVs(quadUVs[3]);
	builder.PushVertex(quadPoints[3]);

	builder.AddQuad(startIndex + 0, startIndex + 1, startIndex + 2, startIndex + 3);


	builder.Finalize();
	quad = builder.CreateMesh(Vertex_3DPCU::GetLayout());
	return quad;
}



Mesh* CreateCube( const Vector3& center, const Vector3& halfDimensions, const RGBA& color, const AABB2D& topUVs, const AABB2D& sideUVs, const AABB2D& bottomUVs)
{
	Vector3 cubePoints[8];
	cubePoints[0] = center + Vector3(-halfDimensions.x, -halfDimensions.y, -halfDimensions.z);
	cubePoints[1] = center + Vector3(+halfDimensions.x, -halfDimensions.y, -halfDimensions.z);
	cubePoints[2] = center + Vector3(+halfDimensions.x, +halfDimensions.y, -halfDimensions.z);
	cubePoints[3] = center + Vector3(-halfDimensions.x, +halfDimensions.y, -halfDimensions.z);

	cubePoints[4] = center + Vector3(-halfDimensions.x, -halfDimensions.y, +halfDimensions.z);
	cubePoints[5] = center + Vector3(+halfDimensions.x, -halfDimensions.y, +halfDimensions.z);
	cubePoints[6] = center + Vector3(+halfDimensions.x, +halfDimensions.y, +halfDimensions.z);
	cubePoints[7] = center + Vector3(-halfDimensions.x, +halfDimensions.y, +halfDimensions.z);


	MeshBuilder builder;
	builder.Initialize(Renderer::PRIMITIVE_TRIANGLES, true);
	builder.SetVertexColor(color); 


	// 012 023 
	builder.SetVertexNormal(0.0f, 0.0f, -1.0f);
	builder.SetVertexTangent(1.0f, 0.0f, 0.0f, 1.0f);

	builder.SetVertexUVs(sideUVs.mins.x, sideUVs.mins.y);
	unsigned int idx = builder.PushVertex(cubePoints[0]); 

	builder.SetVertexUVs(sideUVs.maxs.x, sideUVs.mins.y);
	builder.PushVertex(cubePoints[1]); 

	builder.SetVertexUVs(sideUVs.maxs.x, sideUVs.maxs.y);
	builder.PushVertex(cubePoints[2]); 

	builder.SetVertexUVs(sideUVs.mins.x, sideUVs.maxs.y);
	builder.PushVertex(cubePoints[3]); 

	builder.AddTriangle(idx + 0, idx + 1, idx + 2); 
	builder.AddTriangle(idx + 0, idx + 2, idx + 3); 


	// 156 162
	builder.SetVertexNormal(1.0f, 0.0f, 0.0f);
	builder.SetVertexTangent(0.0f, 0.0f, 1.0f, 1.0f);

	builder.SetVertexUVs(sideUVs.mins.x, sideUVs.mins.y);
	idx = builder.PushVertex(cubePoints[1]); 

	builder.SetVertexUVs(sideUVs.maxs.x, sideUVs.mins.y);
	builder.PushVertex(cubePoints[5]); 

	builder.SetVertexUVs(sideUVs.maxs.x, sideUVs.maxs.y);
	builder.PushVertex(cubePoints[6]); 

	builder.SetVertexUVs(sideUVs.mins.x, sideUVs.maxs.y);
	builder.PushVertex(cubePoints[2]); 

	builder.AddTriangle(idx + 0, idx + 1, idx + 2); 
	builder.AddTriangle(idx + 0, idx + 2, idx + 3); 


	// 547 576
	builder.SetVertexNormal(0.0f, 0.0f, 1.0f);
	builder.SetVertexTangent(-1.0f, 0.0f, 0.0f, 1.0f);

	builder.SetVertexUVs(sideUVs.mins.x, sideUVs.mins.y);
	idx = builder.PushVertex(cubePoints[5]); 

	builder.SetVertexUVs(sideUVs.maxs.x, sideUVs.mins.y);
	builder.PushVertex(cubePoints[4]); 

	builder.SetVertexUVs(sideUVs.maxs.x, sideUVs.maxs.y);
	builder.PushVertex(cubePoints[7]); 

	builder.SetVertexUVs(sideUVs.mins.x, sideUVs.maxs.y);
	builder.PushVertex(cubePoints[6]); 

	builder.AddTriangle(idx + 0, idx + 1, idx + 2); 
	builder.AddTriangle(idx + 0, idx + 2, idx + 3); 


	// 403 437
	builder.SetVertexNormal(-1.0f, 0.0f, 0.0f);
	builder.SetVertexTangent(0.0f, 0.0f, -1.0f, 1.0f);

	builder.SetVertexUVs(sideUVs.mins.x, sideUVs.mins.y);
	idx = builder.PushVertex(cubePoints[4]); 

	builder.SetVertexUVs(sideUVs.maxs.x, sideUVs.mins.y);
	builder.PushVertex(cubePoints[0]); 

	builder.SetVertexUVs(sideUVs.maxs.x, sideUVs.maxs.y);
	builder.PushVertex(cubePoints[3]); 

	builder.SetVertexUVs(sideUVs.mins.x, sideUVs.maxs.y);
	builder.PushVertex(cubePoints[7]); 

	builder.AddTriangle(idx + 0, idx + 1, idx + 2); 
	builder.AddTriangle(idx + 0, idx + 2, idx + 3); 


	// Top
	// 326 367
	builder.SetVertexNormal(0.0f, 1.0f, 0.0f);
	builder.SetVertexTangent(1.0f, 0.0f, 0.0f, 1.0f);

	builder.SetVertexUVs(topUVs.mins.x, topUVs.mins.y);
	idx = builder.PushVertex(cubePoints[3]); 

	builder.SetVertexUVs(topUVs.maxs.x, topUVs.mins.y);
	builder.PushVertex(cubePoints[2]); 

	builder.SetVertexUVs(topUVs.maxs.x, topUVs.maxs.y);
	builder.PushVertex(cubePoints[6]); 

	builder.SetVertexUVs(topUVs.mins.x, topUVs.maxs.y);
	builder.PushVertex(cubePoints[7]); 

	builder.AddTriangle(idx + 0, idx + 1, idx + 2); 
	builder.AddTriangle(idx + 0, idx + 2, idx + 3); 


	// Bottom
	// 045 051
	builder.SetVertexNormal(0.0f, -1.0f, 0.0f);
	builder.SetVertexTangent(1.0f, 0.0f, 0.0f, 1.0f);

	builder.SetVertexUVs(bottomUVs.mins.x, bottomUVs.mins.y);
	idx = builder.PushVertex(cubePoints[0]); 

	builder.SetVertexUVs(bottomUVs.maxs.x, bottomUVs.mins.y);
	builder.PushVertex(cubePoints[4]); 

	builder.SetVertexUVs(bottomUVs.maxs.x, bottomUVs.maxs.y);
	builder.PushVertex(cubePoints[5]); 

	builder.SetVertexUVs(bottomUVs.mins.x, bottomUVs.maxs.y);
	builder.PushVertex(cubePoints[1]); 

	builder.AddTriangle(idx + 0, idx + 1, idx + 2); 
	builder.AddTriangle(idx + 0, idx + 2, idx + 3); 


	builder.Finalize(); 
	return builder.CreateMesh(Vertex_3DPCUTBN::GetLayout()); 
}



Mesh* CreateUVSphere( const Vector3& center, float radius, unsigned int wedges, unsigned int slices, const RGBA& color ) 
{
	// (0,0) to (1,1)

	MeshBuilder builder; 
	builder.Initialize(Renderer::PRIMITIVE_TRIANGLES, true);

	builder.SetVertexColor(color);

	for (unsigned int slice_idx = 0; slice_idx <= slices; ++slice_idx ) {
		float v = (float)slice_idx / (float)slices;
		float azimuth = RangeMapFloat( v, 0.0f, 1.0f, -90.0f, 90.0f ); 

		for (unsigned int wedge_idx = 0; wedge_idx <= wedges; ++wedge_idx ) {
			float u = (float)wedge_idx / (float)wedges; 
			float rot = 360.0f * u; 

			builder.SetVertexUVs( u , v ); 
			Vector3  pos = center + ConvertSphericalToCartesian( radius, rot, azimuth ); 
			builder.SetVertexNormal((pos - center).GetNormalized());
			builder.SetVertexTangent(SinDegrees(rot), 0.0f, CosDegrees(rot), 1.0f);
			builder.PushVertex( pos ); 
		}
	}

	// Left hand rotation
	for (unsigned int slice_idx = 0; slice_idx < slices; ++slice_idx ) {
		for (unsigned int wedge_idx = 0; wedge_idx < wedges; ++wedge_idx ) {
			unsigned int bl_idx = (wedges + 1) * slice_idx + wedge_idx; 
			unsigned int tl_idx = bl_idx + wedges + 1; 
			unsigned int br_idx = bl_idx + 1; 
			unsigned int tr_idx = br_idx + wedges + 1; 

			builder.AddQuad( br_idx, bl_idx, tl_idx, tr_idx ); 
		}
	}

	builder.Finalize();
	return builder.CreateMesh(Vertex_3DPCUTBN::GetLayout()); 
}



Mesh* CreateXYPlane( const Vector3& center, const Vector2& halfDimensions)
{
	Mesh* mesh = CreateCube(center, Vector3(halfDimensions.x, halfDimensions.y, 0.0f));
	return mesh;
}



Mesh* CreatePlane( const Vector3& center, const Vector2& halfDimensions, const Vector3& right, const Vector3& up)
{
	MeshBuilder builder;
	builder.Initialize(Renderer::PRIMITIVE_TRIANGLES, true);

	Vector3 planePoints[4];
	planePoints[0] = center + (right * -halfDimensions.x) + (up * -halfDimensions.y);
	planePoints[1] = center + (right * +halfDimensions.x) + (up * -halfDimensions.y);
	planePoints[2] = center + (right * +halfDimensions.x) + (up * +halfDimensions.y);
	planePoints[3] = center + (right * -halfDimensions.x) + (up * +halfDimensions.y);

	builder.SetVertexUVs(0.0f, 0.0f);
	unsigned int index = builder.PushVertex(planePoints[0]);

	builder.SetVertexUVs(1.0f, 0.0f);
	builder.PushVertex(planePoints[1]);

	builder.SetVertexUVs(1.0f, 1.0f);
	builder.PushVertex(planePoints[2]);

	builder.SetVertexUVs(0.0f, 1.0f);
	builder.PushVertex(planePoints[3]);

	builder.AddQuad(index + 0, index + 1, index + 2, index + 3);


	builder.Finalize();
	Mesh* plane = builder.CreateMesh(Vertex_3DPCUTBN::GetLayout());
	return plane;
}



Mesh* CreateLineSegment( const Vector3& point0, const RGBA& point0Color, const Vector3& point1, const RGBA& point1Color)
{
	// Mesh
	MeshBuilder builder = MeshBuilder();
	builder.Initialize(Renderer::PRIMITIVE_LINES, false);

	builder.SetVertexUVs(0.0f, 0.0f);

	builder.SetVertexColor(point0Color);
	builder.PushVertex(point0);

	builder.SetVertexColor(point1Color);
	builder.PushVertex(point1);

	builder.Finalize();
	Mesh* lineSegment = builder.CreateMesh();

	return lineSegment;
}



Mesh* CreateWireCube( const Vector3& center, const Vector3& halfDimensions, const RGBA& color)
{
	Vector3 cubePoints[8];
	cubePoints[0] = center + Vector3(-halfDimensions.x, -halfDimensions.y, -halfDimensions.z);
	cubePoints[1] = center + Vector3(+halfDimensions.x, -halfDimensions.y, -halfDimensions.z);
	cubePoints[2] = center + Vector3(+halfDimensions.x, +halfDimensions.y, -halfDimensions.z);
	cubePoints[3] = center + Vector3(-halfDimensions.x, +halfDimensions.y, -halfDimensions.z);

	cubePoints[4] = center + Vector3(-halfDimensions.x, -halfDimensions.y, +halfDimensions.z);
	cubePoints[5] = center + Vector3(+halfDimensions.x, -halfDimensions.y, +halfDimensions.z);
	cubePoints[6] = center + Vector3(+halfDimensions.x, +halfDimensions.y, +halfDimensions.z);
	cubePoints[7] = center + Vector3(-halfDimensions.x, +halfDimensions.y, +halfDimensions.z);


	MeshBuilder builder;
	builder.Initialize(Renderer::PRIMITIVE_LINES, true);
	builder.SetVertexColor(color); 
	builder.SetVertexUVs(0.0f, 0.0f);
	builder.SetVertexNormal(0.0f, 0.0f, 0.0f);

	int index = builder.PushVertex(cubePoints[0]);
	builder.PushVertex(cubePoints[1]);
	builder.PushVertex(cubePoints[2]);
	builder.PushVertex(cubePoints[3]);

	builder.PushVertex(cubePoints[4]);
	builder.PushVertex(cubePoints[5]);
	builder.PushVertex(cubePoints[6]);
	builder.PushVertex(cubePoints[7]);

	// Add front/back faces
	builder.AddLineQuad(index + 0, index + 1, index + 2, index + 3);
	builder.AddLineQuad(index + 4, index + 5, index + 6, index + 7);

	// Connect the two faces
	builder.AddLine(index + 0, index + 4);
	builder.AddLine(index + 1, index + 5);
	builder.AddLine(index + 2, index + 6);
	builder.AddLine(index + 3, index + 7);


	builder.Finalize(); 
	return builder.CreateMesh(Vertex_3DPCU::GetLayout()); 
}



Mesh* CreateWireSphere( const Vector3& center, float radius, unsigned int wedges, unsigned int slices, const RGBA& color)
{
	// (0,0) to (1,1)

	MeshBuilder builder; 
	builder.Initialize(Renderer::PRIMITIVE_LINES, true);

	builder.SetVertexColor(color);
	builder.SetVertexNormal(0.0f, 0.0f, 0.0f);

	for (unsigned int slice_idx = 0; slice_idx <= slices; ++slice_idx ) {
		float v = (float)slice_idx / (float)slices;
		float azimuth = RangeMapFloat( v, 0.0f, 1.0f, -90.0f, 90.0f ); 

		for (unsigned int wedge_idx = 0; wedge_idx <= wedges; ++wedge_idx ) {
			float u = (float)wedge_idx / (float)wedges; 
			float rot = 360.0f * u; 

			builder.SetVertexUVs( u , v ); 
			Vector3  pos = center + ConvertSphericalToCartesian( radius, rot, azimuth ); 
			builder.PushVertex( pos ); 
		}
	}

	// Left hand rotation
	for (unsigned int slice_idx = 0; slice_idx < slices; ++slice_idx ) {
		for (unsigned int wedge_idx = 0; wedge_idx < wedges; ++wedge_idx ) {
			unsigned int bl_idx = (wedges + 1) * slice_idx + wedge_idx; 
			unsigned int tl_idx = bl_idx + wedges + 1; 
			unsigned int br_idx = bl_idx + 1; 

			builder.AddLine( br_idx, bl_idx );
			builder.AddLine( bl_idx, tl_idx );
		}
	}

	builder.Finalize();
	return builder.CreateMesh(Vertex_3DPCU::GetLayout()); 
}



Mesh* CreateClusterFromOBJ( const std::string& filename )
{

	// Parse file
	std::string   line;
	std::ifstream file(filename);
	if(!file.is_open())
	{
		return nullptr;
	}

	std::vector<Vector3> positions;
	std::vector<Vector2> uvs;
	std::vector<Vector3> normals;
	std::vector<unsigned int> indices_position;
	std::vector<unsigned int> indices_uv;
	std::vector<unsigned int> indices_normal;
	std::vector<unsigned int> subMeshFinalIndices;
	while(getline(file,line))
	{
		std::vector<std::string> substrings = ParseTokenStringToStrings(line, " ");
		if(substrings.size() == 0)
		{
			continue;
		}

		if(substrings[0] == "v")
		{
			// position
			Vector3 position;
			size_t numElements = substrings.size() - 1;
			if (numElements >= 3)
			{
				position.x = StringToFloat(substrings[1].c_str());
				position.y = StringToFloat(substrings[2].c_str());
				position.z = StringToFloat(substrings[3].c_str());
			}
			positions.push_back(position);
		}
		else if (substrings[0] == "vt")
		{
			// texture coords
			Vector2 uv;
			size_t numElements = substrings.size() - 1;
			if (numElements >= 2)
			{
				uv.x = StringToFloat(substrings[1].c_str());
				uv.y = StringToFloat(substrings[2].c_str());
			}
			uvs.push_back(uv);
		}
		else if (substrings[0] == "vn")
		{
			// normal
			Vector3 normal;
			size_t numElements = substrings.size() - 1;
			if (numElements >= 3)
			{
				normal.x = -StringToFloat(substrings[1].c_str());
				normal.y = StringToFloat(substrings[2].c_str());
				normal.z = StringToFloat(substrings[3].c_str());
			}
			normals.push_back(normal);
		}
		else if (substrings[0] == "f")
		{
			// face
			size_t numElements = substrings.size() - 1;
			std::vector<unsigned int> temp_positions;
			std::vector<unsigned int> temp_uvs;
			std::vector<unsigned int> temp_normals;
			if (numElements == 3)
			{
				// Triangle
				std::vector<std::string> indices = ParseTokenStringToStrings(substrings[1], "/");
				indices_position.push_back(StringToInt(indices[0].c_str()));
				indices_uv.push_back(StringToInt(indices[1].c_str()));
				indices_normal.push_back(StringToInt(indices[2].c_str()));

				indices = ParseTokenStringToStrings(substrings[2], "/");
				indices_position.push_back(StringToInt(indices[0].c_str()));
				indices_uv.push_back(StringToInt(indices[1].c_str()));
				indices_normal.push_back(StringToInt(indices[2].c_str()));

				indices = ParseTokenStringToStrings(substrings[3], "/");
				indices_position.push_back(StringToInt(indices[0].c_str()));
				indices_uv.push_back(StringToInt(indices[1].c_str()));
				indices_normal.push_back(StringToInt(indices[2].c_str()));
			}
			else if (numElements == 4)
			{
				// Quad
				// 1 2 3
				std::vector<std::string> indices = ParseTokenStringToStrings(substrings[1], "/");
				indices_position.push_back(StringToInt(indices[0].c_str()));
				indices_uv.push_back(StringToInt(indices[1].c_str()));
				indices_normal.push_back(StringToInt(indices[2].c_str()));

				indices = ParseTokenStringToStrings(substrings[2], "/");
				indices_position.push_back(StringToInt(indices[0].c_str()));
				indices_uv.push_back(StringToInt(indices[1].c_str()));
				indices_normal.push_back(StringToInt(indices[2].c_str()));

				indices = ParseTokenStringToStrings(substrings[3], "/");
				indices_position.push_back(StringToInt(indices[0].c_str()));
				indices_uv.push_back(StringToInt(indices[1].c_str()));
				indices_normal.push_back(StringToInt(indices[2].c_str()));

				// 1 3 4 
				indices = ParseTokenStringToStrings(substrings[1], "/");
				indices_position.push_back(StringToInt(indices[0].c_str()));
				indices_uv.push_back(StringToInt(indices[1].c_str()));
				indices_normal.push_back(StringToInt(indices[2].c_str()));

				indices = ParseTokenStringToStrings(substrings[3], "/");
				indices_position.push_back(StringToInt(indices[0].c_str()));
				indices_uv.push_back(StringToInt(indices[1].c_str()));
				indices_normal.push_back(StringToInt(indices[2].c_str()));

				indices = ParseTokenStringToStrings(substrings[4], "/");
				indices_position.push_back(StringToInt(indices[0].c_str()));
				indices_uv.push_back(StringToInt(indices[1].c_str()));
				indices_normal.push_back(StringToInt(indices[2].c_str()));
			}
		}
		else if (substrings[0] == "usemtl")
		{
			unsigned int lastIndex = indices_position.size();
			if (lastIndex == 0)
			{
				// Continue
				//		No verts have been pushed yet so discard
			}
			else if (subMeshFinalIndices.size() > 0 && subMeshFinalIndices[subMeshFinalIndices.size() - 1] == lastIndex)
			{
				// Continue
				//		No this submesh has no verts so discard
			}
			else
			{
				subMeshFinalIndices.push_back(lastIndex);
			}
		}
	}
	subMeshFinalIndices.push_back(indices_position.size());
	file.close();


	std::vector<SubMesh*> meshes;
	for (int meshNumber = 0; meshNumber < (int)subMeshFinalIndices.size(); ++meshNumber)
	{
		SubMesh* mesh = nullptr;
		MeshBuilder meshBuilder;
		meshBuilder.Initialize(Renderer::PRIMITIVE_TRIANGLES, false);
		meshBuilder.SetVertexColor(RGBA());

		size_t startIndex = (meshNumber == 0 ? 0 : subMeshFinalIndices[meshNumber - 1]);
		size_t endIndex = subMeshFinalIndices[meshNumber];

		std::vector<Vertex_3DPCUTBN> vertices;
		for (size_t i = startIndex; i < endIndex; ++i)
		{
			unsigned int positionIndex = indices_position[i] - 1;
			unsigned int normalIndex   = indices_normal[i] - 1;
			unsigned int uvIndex	   = indices_uv[i] - 1;
			meshBuilder.SetVertexNormal(normals[normalIndex]);
			meshBuilder.SetVertexUVs(uvs[uvIndex]);
			Vector3 flippedPosition = positions[positionIndex];
			flippedPosition.x = -flippedPosition.x;
			meshBuilder.PushVertex(flippedPosition);
		}

		meshBuilder.GenerateTangents();

		meshBuilder.Finalize();
		mesh = meshBuilder.CreateSubMesh(Vertex_3DPCUTBN::GetLayout());
		meshes.push_back(mesh);
	}

	Mesh* cluster = new Mesh();
	cluster->AddSubMeshes(meshes);
	return cluster;
}



#include "Engine/Rendering/BitmapFont.hpp"
void AddTextToMeshBuilder(MeshBuilder& builder, const Vector3& position, const Vector3& right, const Vector3& up, const RGBA& color, const BitmapFont* font, float fontHeight, float fontAspectScale, const std::string& asciiText, float percentageComplete)
{
	// Short Circuit
	if (font == nullptr || 
		asciiText.length() == 0)
	{
		return;
	}



	// Determine number of visible characters from specified percentage
	float percentagePerCharacter = 1.0f;
	if (asciiText.size() > 0) { percentagePerCharacter = 1.0f / (float)asciiText.size(); }
	float numFadedCharacters = percentageComplete / percentagePerCharacter;
	int numFullyFadedInCharacters = (int)(numFadedCharacters);

	float glyphHeight = fontHeight;
	float glyphWidth  = glyphHeight * (font->GetGlyphAspect(asciiText[0]) * fontAspectScale);
	RGBA glyphColor = color;

	Vector3 textMins = position;
	float glyphHalfWidth = glyphWidth * 0.5f;
	float glyphHalfHeight = glyphHeight * 0.5f;
	for (size_t charIndex = 0; charIndex < asciiText.length(); ++charIndex)
	{
		if ((int)charIndex == numFullyFadedInCharacters)
		{
			glyphColor.a = Interpolate((unsigned char)0, (unsigned char)255, numFadedCharacters - numFullyFadedInCharacters);
		}
		else if ((int)charIndex > numFullyFadedInCharacters)
		{
			glyphColor.a = 0;
		}
		builder.SetVertexColor(glyphColor);

		Vector3 center = textMins;
		center.x += glyphHalfWidth;
		center.y += glyphHalfHeight;

		AABB2D glyphUVs = font->GetUVsForGlyph(asciiText[charIndex]);
		builder.AddBillboardedQuad(center, right, up, Vector2(glyphHalfWidth, glyphHalfHeight), glyphUVs.mins, glyphUVs.maxs, glyphColor);

		textMins += ((glyphWidth) * right);
	}
}



void AddTextToMeshBuilder(MeshBuilder& builder, const Vector2& position, const RGBA& color, const BitmapFont* font, float fontHeight, float fontAspectScale, const std::string& asciiText, float percentageComplete)
{
	AddTextToMeshBuilder(builder, Vector3(position, 0.0f), Vector3::RIGHT, Vector3::UP, color, font, fontHeight, fontAspectScale, asciiText, percentageComplete);
}