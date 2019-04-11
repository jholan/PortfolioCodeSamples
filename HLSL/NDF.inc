#ifndef __NDF_HLSL__
#define __NDF_HLSL__

/* 
	Shader Key
	_LS -> Local Space
	_WS -> World Space
	_VS -> View Space
	_CS -> Clip Space
*/
#include "Constants.inc"

float NormalDistributionFunction_ModifiedTrowbridgeReitz(float3 normal, float3 halfVector, float roughness)
{
	float roughnessSquared = roughness * roughness;	// Disney squared
	roughnessSquared *= roughnessSquared;
	float nDotH = max(dot(normal, halfVector), 0.01f);
	float denominator = (nDotH * nDotH) * (roughnessSquared - 1.0f) + 1.0f;
	float percentageOfMicrofacetsFacingH = roughnessSquared / (PI * (denominator * denominator));
	
	return percentageOfMicrofacetsFacingH;
}

float NormalDistributionFunction_ModifiedTrowbridgeReitz_SphereAreaLight(float3 normal, float3 halfVector, float roughness, float distanceToLight, float radius)
{
	float alpha = roughness;
	float alphaPrime = saturate((radius / (2.0f * distanceToLight)));

	float nf = saturate(pow((alpha/alphaPrime), 2));

	//alpha = alpha / alphaPrime;
	float alphaSquared = alpha * alpha;
	float nDotH = max(dot(normal, halfVector), 0.01f);
	float denominator = (nDotH * nDotH) * (alphaSquared - 1.0f) + 1.0f;
	
	float percentageOfMicrofacetsFacingH = alphaSquared / (PI * (denominator * denominator));
	//percentageOfMicrofacetsFacingH /= nf;
	
	
	
	return percentageOfMicrofacetsFacingH;
}

#endif