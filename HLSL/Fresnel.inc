#ifndef __FRESNEL_HLSL__
#define __FRESNEL_HLSL__

/* 
	Shader Key
	_LS -> Local Space
	_WS -> World Space
	_VS -> View Space
	_CS -> Clip Space
*/
float3 Fresnel_Schlick(float3 normal, float3 directionToEye, float3 F0)
{
	float nDotE = saturate(dot(normal, directionToEye));
	
	float3 reflectance = F0 + ((float3)1.0f - F0) * pow(1.0f - nDotE, 5.0f);
	
	return max(reflectance, (float3)0.01f);
}

// For disney diffuse
float3 Fresnel_SchlickF90(float3 normal, float3 directionToEye, float3 F0, float F90)
{
	float nDotE = saturate(dot(normal, directionToEye));
	
	float3 reflectance = F0 + saturate(((float3)F90 - F0) * pow(1.0f - nDotE, 5.0f));
	
	return max(reflectance, (float3)0.01f);
}

// For IBL
float3 FresnelSchlickRoughness(float3 normal, float3 directionToEye, float3 F0, float roughness)
{
	float cosTheta = saturate(dot(normal, directionToEye));
    return F0 + (max((float3)(1.0f - roughness), F0) - F0) * pow(1.0f - cosTheta, 5.0f);
}   

#endif