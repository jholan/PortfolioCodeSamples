#ifndef __BRDF_HLSL__
#define __BRDF_HLSL__
/* 
	Shader Key
	_LS -> Local Space
	_WS -> World Space
	_VS -> View Space
	_CS -> Clip Space
*/
#include "Constants.inc"

#include "SurfaceProperties.inc"

#include "Fresnel.inc"
#include "NDF.inc"
#include "Geometry.inc"

#define MIN_ROUGHNESS 0.08f
#define MAX_ROUGHNESS 0.999f


float3 EvaluateBRDF(SurfaceProperties surface, float3 luminanceIn, float3 directionToLight, float3 directionToEye, float3 worldNormal)
{
	float3 albedo	= surface.albedo;
	float metalness = surface.metalness;
	float roughness = surface.roughness;
	float3 F0		= surface.F0;
	
	float3 halfVector = normalize(directionToLight + directionToEye);
	
	// Specular Reflectance ------------------------------------------------------------------------------------------------------------
	// Cook-Torrance BRDF
	// Fresnel(F)
	float3 fresnel = Fresnel_Schlick(halfVector, directionToEye, F0);									// Get the reflectance of the microfacets that are able to reflect light into our eye
	
	// Normal Distribution Function(D)
	float ndf = NormalDistributionFunction_ModifiedTrowbridgeReitz(worldNormal, halfVector, roughness);	// Compute the percentage of microfacets facing the halfVector
	
	// Geometry (G)
	float geometry = Geometry_Smith(worldNormal, directionToEye, directionToLight, roughness * roughness);// Compute how much of the surface is visible(Is not being shadowed or masked)  

	// Do the BRDF things
	float nDotE = max(dot(worldNormal, directionToEye), 0.0f);
	float nDotL = max(dot(worldNormal, directionToLight), 0.0f);
	float3  specularReflectance = float3(1.0f, 1.0f, 1.0f) * (fresnel * ndf * geometry) / max(0.001f, 4.0f * nDotE * nDotL);			// Cook-Torrance BRDF Calculation
	
	
	// Diffuse Reflectance --------------------------------------------------------------------------------------------------------------
	// Calculate how much light is refracted
	float3 percentageReflected = fresnel;
	float3 percentageRefracted = float3(1.0f, 1.0f, 1.0f) - percentageReflected;						// The remaining light that was not reflected will be refracted 
	percentageRefracted *= (1.0f - metalness);															// Metal surfaces absorb ALL refracted light
	
	
	// Lambertian BRDF
	float3 diffuseReflectance = percentageRefracted * (albedo / PI);
	
		
	// Reflectance Equation
	float3 luminanceOut = (diffuseReflectance + specularReflectance) * luminanceIn; // If we are doing nDotL here is this not illuminanceIn?
	return luminanceOut;
}

#endif