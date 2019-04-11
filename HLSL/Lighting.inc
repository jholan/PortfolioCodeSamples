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



// Utility Functions ---------------------------------------------------------------------------------------
// A right disk is a disk oriented to always face the lit surface .
// Solid angle of a sphere or a right disk is 2 PI (1 - cos( subtended angle )).
// Subtended angle sigma = arcsin (r / d) for a sphere
// and sigma = atan (r / d) for a right disk
// sinSigmaSqr = sin( subtended angle )^2 , it is (r^2 / d^2) for a sphere
// and (r^2 / ( r^2 + d ^2) ) for a disk
// cosTheta is not clamped
float illuminanceSphereOrDisk(float cosTheta , float sinSigmaSqr)
{
	float sinTheta = sqrt (1.0f - cosTheta * cosTheta );
	
	float illuminance = 0.0f ;
	// Note : Following test is equivalent to the original formula .
	// There is 3 phase in the curve : cosTheta > sqrt ( sinSigmaSqr ),
	// cosTheta > -sqrt ( sinSigmaSqr ) and else it is 0
	// The two outer case can be merge into a cosTheta * cosTheta > sinSigmaSqr
	// and using saturate ( cosTheta ) instead .
	if ( cosTheta * cosTheta > sinSigmaSqr )
	{
		illuminance = PI * sinSigmaSqr * saturate ( cosTheta );
	}
	else
	{
		float x = sqrt(1.0f / sinSigmaSqr - 1.0f) ; // For a disk this simplify to x = d / r
		float y = -x * ( cosTheta / sinTheta );
		float sinThetaSqrtY = sinTheta * sqrt (1.0f - y * y);
		illuminance = ( cosTheta * acos( y) - x * sinThetaSqrtY ) * sinSigmaSqr + atan(sinThetaSqrtY / x);
	}

	return max ( illuminance , 0.0f);
}



// Light Evaluation -----------------------------------------------------------------------------------------
float3 EvaluatePointLight(SurfaceProperties surface, PointLightData light, float3 eyePosition_WS)
{
	float3 directionToEye = normalize(eyePosition_WS - surface.position_WS);
	float3 directionToLight = normalize(light.m_position_WS - surface.position_WS);
	
	// Attenuation
	float attenuation = GetAttenuationForLight_Dice(light.m_position_WS, surface.position_WS, light.m_attenuationRadius);
	
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
	float attenuation = GetAttenuationForLight_Dice(light.m_position, surface.position_WS, light.m_attenuationRadius);
	
	// Calculate incoming Luminance(L_i)
	float luminousPower = light.m_colorAndPower.a;
	float luminousIntensity = luminousPower / PI;
	//float luminousIntensity = luminousPower / ( 2.0f * PI * (1.0f - light.m_spotLightOuterCosAngle));
	float3 luminanceIn = (light.m_colorAndPower.rgb * luminousIntensity) * attenuation * saturate(dot(surface.normal_WS, directionToLight)); 
	
	// Calculate outgoing Luminance(L_o)
	float3 luminanceOut = EvaluateBRDF(surface, luminanceIn, directionToLight, directionToEye, surface.normal_WS) * coneFactor;
	
	return luminanceOut;
}


float3 EvaluateSphereLight(SurfaceProperties surface, SphereLightData light, float3 eyePosition_WS)
{
	float3 directionToEye = normalize(eyePosition_WS - surface.position_WS);

	// Constants
	float3	lightPosition = light.m_position_WS;
	float 	lightRadius = light.m_radius;
	float 	sqrLightRadius = lightRadius * lightRadius;
	float3 	lightColor = light.m_colorAndPower.rgb;
	float 	luminousPower = light.m_colorAndPower.w;
	float 	lightLuminance = luminousPower / (4.0f * PI * PI * sqrLightRadius);
	
	
	// Most representitive point(Specular highlights)
	// Light Direction
	float3 	L = lightPosition - surface.position_WS;
	float3 	r = reflect(-directionToEye, surface.normal_WS);
	float3 	L_on_r = dot(L,r) * r;
	float3 	lightCenterToLprojected = L_on_r - L;
	float3 	closestPointToROnLight = L + (lightCenterToLprojected * saturate(lightRadius / length(lightCenterToLprojected)));
	float 	distanceToClosestLightPoint = length(closestPointToROnLight);
	float3 	directionToLight = normalize(closestPointToROnLight);
	
	
	// Calculate incoming luminance(L_i)
	// Provides wrapped lighting
	float3  toLightCenter = lightPosition - surface.position_WS;
	float3  directionToLightCenter = normalize(toLightCenter);
	float 	sqrDistanceToLightCenter = dot(toLightCenter , toLightCenter);
	float 	cosTheta = clamp( dot( surface.normal_WS , directionToLightCenter ) , -0.999f , 0.999f) ; // Clamp to avoid edge case	
	float 	sinSigmaSqr = min(sqrLightRadius / sqrDistanceToLightCenter , 0.9999f);
	
	float piFormFactor = PI * sqrLightRadius * saturate(cosTheta) / sqrDistanceToLightCenter ; // Visibility of the light from the surface point * PI(remove the baked /PI from the form factor calc)
	if(sqrDistanceToLightCenter < 100.0f * sqrLightRadius)
	{
		piFormFactor = illuminanceSphereOrDisk(cosTheta , sinSigmaSqr);
	}
	float3 luminanceIn = lightColor * lightLuminance * piFormFactor;
	luminanceIn = luminanceIn;
	
	
	// BRDF Calculation
	// Cached values
	float3  albedo		= surface.albedo;
	float 	metalness 	= surface.metalness;
	float 	roughness 	= surface.roughness * surface.roughness;
	float3  F0			= surface.F0;
	
	float3 halfVector 	= normalize(directionToLight + directionToEye);
	
	float nDotE = max(dot(surface.normal_WS, directionToEye), 0.001f);
	float nDotL = max(dot(surface.normal_WS, directionToLight), 0.001f);
	
	// Normalization factor
	float alpha = roughness;
	float alphaPrime = saturate(alpha + (lightRadius / max(.001, (3.0f * distanceToClosestLightPoint))));
	float sphereNormalization = saturate((alpha / alphaPrime) * (alpha / alphaPrime));
	
	
	// Specular Reflectance ------------------------------------------------------------------------------------------------------------
	// Cook-Torrance BRDF
	float3  fresnel = Fresnel_Schlick(halfVector, directionToEye, F0);

	float ndf = NormalDistributionFunction_ModifiedTrowbridgeReitz_SphereAreaLight(surface.normal_WS, halfVector, roughness, distanceToClosestLightPoint, lightRadius);
	ndf = max(.001, ndf * sphereNormalization);

	float geometry = Geometry_Smith(surface.normal_WS, directionToEye, directionToLight, roughness);			
	
	float3 specularReflectance = float3(1.0f, 1.0f, 1.0f) * (fresnel * ndf * geometry) / max(0.001f, 4.0f * nDotE * nDotL);
	
	
	// Diffuse Reflectance --------------------------------------------------------------------------------------------------------------
	// Calculate how much light is refracted
	float3 percentageReflected = fresnel;
	float3 percentageRefracted = float3(1.0f, 1.0f, 1.0f) - percentageReflected;
	percentageRefracted *= (1.0f - metalness);
	
	// Lambertian BRDF
	float3 diffuseReflectance = percentageRefracted * (albedo / PI);
		
	// Reflectance Equation
	float3 luminanceOutFromLight = (diffuseReflectance + specularReflectance) * luminanceIn; 
	float3 luminanceOut = max(luminanceOutFromLight, 0.0f);
	
	return luminanceOut;
}
