#ifndef __GEOMETRY_HLSL__
#define __GEOMETRY_HLSL__

/* 
	Shader Key
	_LS -> Local Space
	_WS -> World Space
	_VS -> View Space
	_CS -> Clip Space
*/

// direction should be either directionToEye or directionToLight
float Geometry_ModifiedTrowbridgeReitz(float3 normal, float3 direction, float roughness)
{
	float nDotD = max(dot(normal, direction), 0.0f);
	float modifiedRoughness = ((roughness + 1.0f) * (roughness + 1.0f)) / 8.0f;
	
	float percentageOfSurfaceThatIsVisible = nDotD / (nDotD * (1.0f - modifiedRoughness) + modifiedRoughness);
	
	return percentageOfSurfaceThatIsVisible;
}

float Geometry_Smith(float3 normal, float3 directionToEye, float3 directionToLight, float roughness)
{
	return Geometry_ModifiedTrowbridgeReitz(normal, directionToEye, roughness) * Geometry_ModifiedTrowbridgeReitz(normal, directionToLight, roughness);
}

#endif