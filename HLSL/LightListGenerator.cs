/* 
	Shader Key
	_LS -> Local Space
	_WS -> World Space
	_VS -> View Space
	_CS -> Clip Space
	_SS -> Screen Space
*/
#include "SystemConstantBuffers.inc"
#include "Lighting.inc"



// Frustum generation --------------------------------------------------------------
float4 ClipSpaceToViewSpace(float4 position_CS)
{
	float4 position_VS = mul(INVERSE_PROJECTION, position_CS);
	position_VS /= position_VS.w;
	
	return position_VS;
}

float4 ScreenSpaceToViewSpace(float4 position_SS)
{
	float2 uvs = position_SS.xy / SCREEN_DIMENSIONS;
	uvs.y = 1.0f - uvs.y; // flip y
	float4 position_CS = float4(uvs * 2.0f - 1.0f, position_SS.z, position_SS.w);
	float4 position_VS = ClipSpaceToViewSpace(position_CS);
	
	return position_VS;
}

struct Plane
{
	float 	distanceToOrigin;
	float3 	normal;
};

Plane CreatePlane(float3 p0, float3 p1, float3 p2)
{
	Plane plane;
	
	float3 v01 = p1 - p0;
	float3 v02 = p2 - p0;
	
	plane.normal = normalize(cross(v01, v02));
	plane.distanceToOrigin = dot(plane.normal, p0);
	
	return plane;
}

struct Frustum
{
	Plane planes[4]; // Left, Right, Top, Bottom
};



// Sphere Tests -------------------------------------------------------------------------
bool DoesSphereIntersectPlane(float3 sphereCenter, float sphereRadius, Plane plane)
{
	// Project the center of the sphere onto the normal of the plane and account for the plane distance from the origin
	float projectedDist = dot(sphereCenter, plane.normal) - plane.distanceToOrigin;
	float absProjectedDistance = abs(projectedDist);
	
	// If the spheres center is <= a radius from the plane it intersects
	bool intersects = (absProjectedDistance <= sphereRadius);
	return intersects;
}

bool IsSphereBehindPlane(float3 sphereCenter, float sphereRadius, Plane plane)
{
	// Project the center of the sphere onto the normal of the plane and account for the plane distance from the origin
	float projectedDist = dot(sphereCenter, plane.normal) - plane.distanceToOrigin;
	
	// if the projected distance is more than a radius length beneath the plane it is fully behind the plane
	bool isBehind = (projectedDist < -sphereRadius);
	return isBehind;
}

bool IsSphereInsideFrustum(float3 sphereCenter, float sphereRadius, Frustum frustum, float zNear, float zFar )
{
    // First check depth
	// If the farthest point on the sphere along the view direction is not past the near plane OR
	//	  the nearest point of the sphere along the view direction is beyond the the far plane
	// 	  we can't be in the frustum
	bool infrontOfNearClip = sphereCenter.z + sphereRadius < zNear;
	bool behindFarClip = sphereCenter.z - sphereRadius > zFar;
	bool isInside = !(infrontOfNearClip ||
						behindFarClip ||
						IsSphereBehindPlane(sphereCenter, sphereRadius, frustum.planes[0]) ||
						IsSphereBehindPlane(sphereCenter, sphereRadius, frustum.planes[1]) ||
						IsSphereBehindPlane(sphereCenter, sphereRadius, frustum.planes[2]) ||
						IsSphereBehindPlane(sphereCenter, sphereRadius, frustum.planes[3])
					);
	return isInside;
}



// Light lists --------------------------------------------------------------------------
// Defines
#define MAX_INDICES_POINT 	1024
#define MAX_INDICES_SPOT 	768
#define MAX_INDICES_SPHERE 	768
#define MAX_INDICES_TUBE 	768



// Structs --------------------------------------------------
struct TileLightingInfo
{
	int globalIndexListStartIndex_Point;
	int globalIndexListCount_Point;
	
	int globalIndexListStartIndex_Spot;
	int globalIndexListCount_Spot;
	
	int globalIndexListStartIndex_Sphere;
	int globalIndexListCount_Sphere;
	
	int globalIndexListStartIndex_Tube;
	int globalIndexListCount_Tube;
};

struct LightIndexManager
{
	int nextFreeIndex_Point;
	int nextFreeIndex_Spot;
	int nextFreeIndex_Sphere;
	int nextFreeIndex_Tube;
};



// Constants --------------------------------------------------
cbuffer LightListsMetaDataConstantBuffer : register(b7)
{
	int 	NUM_LIGHTS_POINT;
	int 	NUM_LIGHTS_SPOT;
	int 	NUM_LIGHTS_SPHERE;
	int 	NUM_LIGHTS_TUBE;
}

cbuffer PlaneChoiceConstantBuffer : register(b8)
{
	float 	NEAR_PLANE_CHOICE;
	float 	PADDING_b8_0;
    float   PADDING_b8_1;
	float 	PADDING_b8_2;
}

Texture2D<float4> 						DEPTH_TEXTURE 				: register(t0);		// Input

RWTexture2D<float4>						visBuffer					: register(u0);	

RWStructuredBuffer<PointLightData>  	GLOBAL_LIGHT_LIST_POINT 	: register(u2);		// Input
RWStructuredBuffer<SpotLightData>  	    GLOBAL_LIGHT_LIST_SPOT 		: register(u3);		// Input
RWStructuredBuffer<SphereLightData>  	GLOBAL_LIGHT_LIST_SPHERE 	: register(u4);		// Input
RWStructuredBuffer<TubeLightData>  	    GLOBAL_LIGHT_LIST_TUBE 		: register(u5);		// Input

RWStructuredBuffer<LightIndexManager>	globalIndexListManager		: register(u6); 	// Output
RWStructuredBuffer<uint> 				globalIndexList_Point		: register(u7);		// Output
RWStructuredBuffer<uint> 				globalIndexList_Spot		: register(u8);		// Output
RWStructuredBuffer<uint> 				globalIndexList_Sphere		: register(u9);		// Output
RWStructuredBuffer<uint> 				globalIndexList_Tube		: register(u10);	// Output

RWStructuredBuffer<TileLightingInfo>	tileLightingInfo			: register(u15);	// Output



// Group Shared --------------------------------------------------
groupshared uint minDepth_uint;
groupshared uint maxDepth_uint;

groupshared int indices_Point[MAX_INDICES_POINT];
groupshared int numIndices_Point;
groupshared int globalIndexListOffset_Point;

groupshared int indices_Spot[MAX_INDICES_SPOT];
groupshared int numIndices_Spot;
groupshared int globalIndexListOffset_Spot;

groupshared int indices_Sphere[MAX_INDICES_SPHERE];
groupshared int numIndices_Sphere;
groupshared int globalIndexListOffset_Sphere;

groupshared int indices_Tube[MAX_INDICES_TUBE];
groupshared int numIndices_Tube;
groupshared int globalIndexListOffset_Tube;



// Entry Point --------------------------------------------------
static const uint3 THREAD_GROUP_DIMENSIONS = uint3(16, 16, 1);
[numthreads (THREAD_GROUP_DIMENSIONS.x, THREAD_GROUP_DIMENSIONS.y, THREAD_GROUP_DIMENSIONS.z)]
void Main_ComputeShader( 	uint3 threadGroupID		: SV_GroupID,				// Dispatch relative 3D index for a ThreadGroup
							uint3 threadID_TG_3D 	: SV_GroupThreadID,			// ThreadGroup relative 3D index for a thread
							uint  threadID_TG_1D 	: SV_GroupIndex,			// ThreadGroup relative 1D index for a thread			
							uint3 threadID_Dsp_3D 	: SV_DispatchThreadID		// Dispatch relative 3D index for a thread
							)			
{
	float depth = DEPTH_TEXTURE.Load(uint3(threadID_Dsp_3D.xy, 0)).r;
	
	uint numThreadsPerGroup = THREAD_GROUP_DIMENSIONS.x * THREAD_GROUP_DIMENSIONS.y;
	uint threadGroupID_Dsp_1D = threadGroupID.x + ((SCREEN_WIDTH/THREAD_GROUP_DIMENSIONS.x) * threadGroupID.y);

	// Depth ------------------------------------------------------------------------------------------------------------
	if(threadID_TG_1D == 0)
	{
		minDepth_uint = 0x7F7FFFFF;
		maxDepth_uint = 0;
		
		numIndices_Point  = 0;
		numIndices_Spot   = 0;
		numIndices_Sphere = 0;
		numIndices_Tube   = 0;
	}
	GroupMemoryBarrierWithGroupSync(); // Confirm we have tested all the depth
	
	
	// Find the depth range
	if(depth != 0.0f)
	{
		uint depth_uint = asuint(depth); // atomic ops only work on int/uint so we need to convert to save it off
		InterlockedMax(maxDepth_uint, depth_uint);
		InterlockedMin(minDepth_uint, depth_uint);
	}
	
	
	
	// Frustum -----------------------------------------------------------------------------------------------------------
	// 0 -- 1
	// |    |
	// 2 -- 3
	float4 frustumPositions_SS[4];
	frustumPositions_SS[0] = float4((threadGroupID.xy + float2(0,0)) * THREAD_GROUP_DIMENSIONS.xy, 1, 1);
	frustumPositions_SS[1] = float4((threadGroupID.xy + float2(1,0)) * THREAD_GROUP_DIMENSIONS.xy, 1, 1);
	frustumPositions_SS[2] = float4((threadGroupID.xy + float2(0,1)) * THREAD_GROUP_DIMENSIONS.xy, 1, 1);
	frustumPositions_SS[3] = float4((threadGroupID.xy + float2(1,1)) * THREAD_GROUP_DIMENSIONS.xy, 1, 1);
 
 
	float3 frustumPositions_VS[4];
	frustumPositions_VS[0] = ScreenSpaceToViewSpace(frustumPositions_SS[0]).xyz;
	frustumPositions_VS[1] = ScreenSpaceToViewSpace(frustumPositions_SS[1]).xyz;
	frustumPositions_VS[2] = ScreenSpaceToViewSpace(frustumPositions_SS[2]).xyz;
	frustumPositions_VS[3] = ScreenSpaceToViewSpace(frustumPositions_SS[3]).xyz;
 
	
	Frustum tileFrustum;
	float3 eyePosition = float3(0,0,0);
	// Planes face INWARD 
	tileFrustum.planes[0] = CreatePlane(eyePosition, frustumPositions_VS[0], frustumPositions_VS[2]);
	tileFrustum.planes[1] = CreatePlane(eyePosition, frustumPositions_VS[3], frustumPositions_VS[1]);
	tileFrustum.planes[2] = CreatePlane(eyePosition, frustumPositions_VS[1], frustumPositions_VS[0]);
	tileFrustum.planes[3] = CreatePlane(eyePosition, frustumPositions_VS[2], frustumPositions_VS[3]);

	
	
	// Near/ Far Reconstruction ----------------------------------------------------------------------------------------------
	GroupMemoryBarrierWithGroupSync(); // Confirm we have tested all the depths
	
	
	// Grap the min/max depth and cast them back to useful
	float minDepth_CS = asfloat(minDepth_uint);
	float maxDepth_CS = asfloat(maxDepth_uint);
	
	// Convert depth values to view space.
	float minDepth_VS = ClipSpaceToViewSpace(float4(0, 0, minDepth_CS, 1)).z;
	float maxDepth_VS = ClipSpaceToViewSpace(float4(0, 0, maxDepth_CS, 1)).z;
	float nearClip_VS = ClipSpaceToViewSpace(float4(0, 0, 0, 1)).z;
	minDepth_VS = lerp(minDepth_VS, nearClip_VS, NEAR_PLANE_CHOICE);
	
	

    //// Light Culling --------------------------------------------------------------------------------------------------------
	// Point
	for (int i = 0 + threadID_TG_1D; i < NUM_LIGHTS_POINT; i += numThreadsPerGroup)
	{
		PointLightData light = GLOBAL_LIGHT_LIST_POINT[i];
		float3 lightPosition = mul(VIEW, float4(light.m_position_WS, 1.0f)).xyz;
		float  lightRadius = light.m_attenuationRadius;
	
		if(IsSphereInsideFrustum(lightPosition, lightRadius, tileFrustum, minDepth_VS, maxDepth_VS))
		{
			int index;
			InterlockedAdd(numIndices_Point, 1, index);
			if(index < MAX_INDICES_POINT)
			{
				indices_Point[index] = i;
			}
		}
	}
	
	// Spot
	for (int i = 0 + threadID_TG_1D; i < NUM_LIGHTS_SPOT; i += numThreadsPerGroup)
	{
		SpotLightData light = GLOBAL_LIGHT_LIST_SPOT[i];
		float3 lightPosition = mul(VIEW, float4(light.m_position, 1.0f)).xyz;
		float  lightRadius = light.m_attenuationRadius;
	
		if(IsSphereInsideFrustum(lightPosition, lightRadius, tileFrustum, minDepth_VS, maxDepth_VS))
		{ 
			int index;
			InterlockedAdd(numIndices_Spot, 1, index);
			if(index < MAX_INDICES_SPOT)
			{
				indices_Spot[index] = i;
			}
		}
	}
	
	// Sphere 
	for (int i = 0 + threadID_TG_1D; i < NUM_LIGHTS_SPHERE; i += numThreadsPerGroup)
	{
		SphereLightData light = GLOBAL_LIGHT_LIST_SPHERE[i];
		float3 lightPosition = mul(VIEW, float4(light.m_position_WS, 1.0f)).xyz;
		float  lightRadius = light.m_attenuationRadius;
	
		if(IsSphereInsideFrustum(lightPosition, lightRadius, tileFrustum, minDepth_VS, maxDepth_VS))
		{ 
			int index;
			InterlockedAdd(numIndices_Sphere, 1, index);
			if(index < MAX_INDICES_SPHERE)
			{
				indices_Sphere[index] = i;
			}
		}
	}
	
	// Tube
	for (int i = 0 + threadID_TG_1D; i < NUM_LIGHTS_TUBE; i += numThreadsPerGroup)
	{
		TubeLightData light = GLOBAL_LIGHT_LIST_TUBE[i];
		float3 lightPosition_WS = (light.m_position0 + light.m_position1) / 2.0f;
		float3 lightPosition = mul(VIEW, float4(lightPosition_WS, 1.0f)).xyz;
		float  lightRadius = light.m_attenuationRadius;
	
		if(IsSphereInsideFrustum(lightPosition, lightRadius, tileFrustum, minDepth_VS, maxDepth_VS))
		{ 
			int index;
			InterlockedAdd(numIndices_Tube, 1, index);
			if(index < MAX_INDICES_TUBE)
			{
				indices_Tube[index] = i;
			}
		}
	}
	
	GroupMemoryBarrierWithGroupSync();

	
	
	//// Output --------------------------------------------------------------------------------------------------------
	// Reserve our spaces in the global index lists
	// Add the necessary data to the tile lighting info struct to retrieve the indices
	if (threadID_TG_1D == 0)
	{
		InterlockedAdd(globalIndexListManager[0].nextFreeIndex_Point, numIndices_Point, globalIndexListOffset_Point);
		tileLightingInfo[threadGroupID_Dsp_1D].globalIndexListStartIndex_Point = globalIndexListOffset_Point;
		tileLightingInfo[threadGroupID_Dsp_1D].globalIndexListCount_Point = numIndices_Point;
		
		InterlockedAdd(globalIndexListManager[0].nextFreeIndex_Spot, numIndices_Spot, globalIndexListOffset_Spot);
		tileLightingInfo[threadGroupID_Dsp_1D].globalIndexListStartIndex_Spot = globalIndexListOffset_Spot;
		tileLightingInfo[threadGroupID_Dsp_1D].globalIndexListCount_Spot = numIndices_Spot;
		
		InterlockedAdd(globalIndexListManager[0].nextFreeIndex_Sphere, numIndices_Sphere, globalIndexListOffset_Sphere);
		tileLightingInfo[threadGroupID_Dsp_1D].globalIndexListStartIndex_Sphere = globalIndexListOffset_Sphere;
		tileLightingInfo[threadGroupID_Dsp_1D].globalIndexListCount_Sphere = numIndices_Sphere;
		
		InterlockedAdd(globalIndexListManager[0].nextFreeIndex_Tube, numIndices_Tube, globalIndexListOffset_Tube);
		tileLightingInfo[threadGroupID_Dsp_1D].globalIndexListStartIndex_Tube = globalIndexListOffset_Tube;
		tileLightingInfo[threadGroupID_Dsp_1D].globalIndexListCount_Tube = numIndices_Tube;
	}
	
	AllMemoryBarrierWithGroupSync();
	
	
	// Push the indices into the global index lists
	for (int i = 0 + threadID_TG_1D; i < numIndices_Point; i += numThreadsPerGroup)
	{
		globalIndexList_Point[globalIndexListOffset_Point + i] = indices_Point[i];
	}
	
	for (int i = 0 + threadID_TG_1D; i < numIndices_Spot; i += numThreadsPerGroup)
	{
		globalIndexList_Spot[globalIndexListOffset_Spot + i] = indices_Spot[i];
	}
	
	for (int i = 0 + threadID_TG_1D; i < numIndices_Sphere; i += numThreadsPerGroup)
	{
		globalIndexList_Sphere[globalIndexListOffset_Sphere + i] = indices_Sphere[i];
	}
	
	for (int i = 0 + threadID_TG_1D; i < numIndices_Tube; i += numThreadsPerGroup)
	{
		globalIndexList_Tube[globalIndexListOffset_Tube + i] = indices_Tube[i];
	}
	
};