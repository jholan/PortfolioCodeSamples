/* 
	Shader Key
	_LS -> Local Space
	_WS -> World Space
	_VS -> View Space
	_CS -> Clip Space
*/

#include "Constants.inc"
#include "SurfaceProperties.inc"
#include "Attenuation.inc"
#include "BRDF.inc"


// Light Structs -------------------------------------------------------------------------------------------
struct PointLightData
{
	float4	m_colorAndPower; // rgb color, a Luminous Power(lumens)
	
	float3	m_position_WS;
	float	m_attenuationRadius;
};


struct SpotLightData
{
	float4   m_colorAndPower; 	// rgb color, a luminous power

	float3   m_lightDirection;
	float	 m_spotLightInnerCosAngle;

	float3   m_position;
	float	 m_spotLightOuterCosAngle;

	float	 m_attenuationRadius;
	float	 m_PADDING_0;
	float	 m_PADDING_1;
	float	 m_PADDING_2;
	
	float4x4 m_shadowViewProjection;
};


struct SphereLightData
{
	float4	m_colorAndPower; // rgb color, a Luminous Power(lumens)
	
	float3	m_position_WS;
	float	m_attenuationRadius;
	
	float 	m_radius;
	float 	m_PADDING_0;
	float 	m_PADDING_1;
	float 	m_PADDING_2;
};


struct TubeLightData
{
	float4   m_colorAndPower; 	// rgb color, a luminous power(lumens)

	float3   m_position0;
	float	 m_attenuationRadius;

	float3	 m_position1;
	float	 m_radius;
};



// Light Evaluation -----------------------------------------------------------------------------------------
float3 EvaluatePointLight(SurfaceProperties surface, PointLightData light, float3 eyePosition_WS)
{
	float3 directionToEye = normalize(eyePosition_WS - surface.position_WS);
	float3 directionToLight = normalize(light.m_position_WS - surface.position_WS);
	
	// Attenuation
	float attenuation = GetAttenuationForLight(light.m_position_WS, surface.position_WS, light.m_attenuationRadius);
	
	// Calculate incoming Luminance(L_i)
	float luminousPower = light.m_colorAndPower.a;
	float luminousIntensity = luminousPower / (4.0f * PI);
	float3 luminanceIn = (light.m_colorAndPower.rgb * luminousIntensity) * attenuation * saturate(dot(surface.normal_WS, directionToLight)); 
	
	// Calculate outgoing Luminance(L_o)
	float3 luminanceOut = EvaluateBRDF(surface, luminanceIn, directionToLight, directionToEye, surface.normal_WS);
	
	
	return luminanceOut;
}


float3 EvaluateSpotLight(SurfaceProperties surface, SpotLightData light, float3 eyePosition_WS)
{
	// Light Direction
	float3 directionToEye = normalize(eyePosition_WS - surface.position_WS);
	float3 directionToLight = normalize(light.m_position - surface.position_WS);
	
	// Cone Factor
	float cosAngle_LightDir_SurfaceDir = saturate(dot(light.m_lightDirection, -directionToLight));
	float coneFactor = smoothstep(light.m_spotLightOuterCosAngle, light.m_spotLightInnerCosAngle, cosAngle_LightDir_SurfaceDir);
	
	// Attenuation
	float attenuation = GetAttenuationForLight(light.m_position, surface.position_WS, light.m_attenuationRadius);
	
	// Calculate incoming Luminance(L_i)
	float luminousPower = light.m_colorAndPower.a;
	float luminousIntensity = luminousPower / PI; // Masked Hemisphere
	//float luminousIntensity = luminousPower / ( 2.0f * PI * (1.0f - light.m_spotLightOuterCosAngle)); // Reflector
	float3 luminanceIn = (light.m_colorAndPower.rgb * luminousIntensity) * attenuation * saturate(dot(surface.normal_WS, directionToLight)); 
	
	// Calculate outgoing Luminance(L_o)
	float3 luminanceOut = EvaluateBRDF(surface, luminanceIn, directionToLight, directionToEye, surface.normal_WS) * coneFactor;
	
	return luminanceOut;
}
