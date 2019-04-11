#ifndef __ATTENUATION_HLSL__
#define __ATTENUATION_HLSL__

/* 
	Shader Key
	_LS -> Local Space
	_WS -> World Space
	_VS -> View Space
	_CS -> Clip Space
*/
float GetAttenuationForLight_InverseSquare(float3 lightPosition_WS, float3 surfacePosition_WS)
{
	float  dist = length(lightPosition_WS - surfacePosition_WS);
	float attenuation = 1.0f / (dist * dist);
	
	return attenuation;
}

float GetAttenuationForLight_UE4(float3 lightPosition_WS, float3 surfacePosition_WS, float attenuationRadius)
{
	float distance = length(lightPosition_WS - surfacePosition_WS);
	float distanceSquared = distance * distance; 
	float distanceOverLightRadius = distance / attenuationRadius;
	
	float tmpMath = saturate(1.0f - (distanceOverLightRadius * distanceOverLightRadius * distanceOverLightRadius * distanceOverLightRadius));
	float numerator = tmpMath * tmpMath;
	
	float attenuation = numerator / (1.0f + distanceSquared);
	
	return attenuation;
}

float GetAttenuationForLight_Dice(float3 lightPosition_WS, float3 surfacePosition_WS, float attenuationRadius)
{
	float distance = length(lightPosition_WS - surfacePosition_WS);
	float distanceSquared = distance * distance; 
	float distanceToThe4th = distanceSquared * distanceSquared;
	float lightRadiusToThe4th = attenuationRadius * attenuationRadius * attenuationRadius * attenuationRadius;
	
	float window = saturate(1.0f - (distanceToThe4th / lightRadiusToThe4th));
	float attenuation = (1.0f / distanceSquared) * (window * window);
	
	return attenuation;
}


float GetAttenuationForLight(float3 lightPosition_WS, float3 surfacePosition_WS, float attenuationRadius)
{	
	float attenuation = GetAttenuationForLight_Dice(lightPosition_WS, surfacePosition_WS, attenuationRadius);
	
	return attenuation;
}

#endif