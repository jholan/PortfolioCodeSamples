#version 420 core

#define MAX_LIGHTS 8

layout(binding=2, std140) uniform CameraUniformBlock 
{
   mat4 	VIEW; 
   mat4 	PROJECTION; 
   
   vec3 	EYE_POSITION;
   float 	PADDING_0;
}; 

struct LightData
{
	vec4  m_colorAndIntensity;
	
	vec3  m_lightDirection;
	float m_isPointLight; 	// 0 for yes, 1 for no
	
	vec3  m_position;
	float m_spotLightInnerCosAngle;
	
	vec3  m_attenuation;	
	float m_spotLightOuterCosAngle;  
};

layout(binding=4, std140) uniform LightUniformBlock 
{
	vec4 	  AMBIENT;
	LightData LIGHTS[MAX_LIGHTS];
}; 

// Surface data
layout(binding=8, std140) uniform MaterialPropertyBlock0
{
	float SPECULAR_AMOUNT; // shininess (0 to 1)
	float SPECULAR_POWER;  // smoothness (1 to whatever)
};

float GetAttenuationForLight(in int index, in vec3 surfacePoint)
{
	float  dist = length(LIGHTS[index].m_position - surfacePoint);
	vec3   distances = vec3(1.0f, dist, dist * dist);
	float  attenuation = LIGHTS[index].m_colorAndIntensity.a / dot(LIGHTS[index].m_attenuation, distances);
	return attenuation;
}

// Uniforms ----------------------------------------------


// Textures
layout(binding = 0) uniform sampler2D gTexDiffuse;
layout(binding = 1) uniform sampler2D gTexNormal; 


// Inputs ----------------------------------------------
in vec4 passColor;
in vec2 passUV; 
in vec3 passWorldNormal;
in vec3 passWorldPosition;
in vec3 passWorldTangent;
in vec3 passWorldBitangent;


// Outputs ----------------------------------------------
out vec4 outColor; 



void main( void )
{
	vec4 surfaceColor = texture( gTexDiffuse, passUV ); 
	vec4 normalMapColor = texture( gTexNormal, passUV );

	vec3 vertexNormal = normalize(passWorldNormal);   	// Renormaize because of interpolation
	vec3 vertexTangent = normalize(passWorldTangent);
	vec3 vertexBitangent = normalize(passWorldBitangent);
	mat3 surfaceToWorld = mat3(vertexTangent, vertexBitangent, vertexNormal);
	
	vec3 surfaceNormal = normalize( normalMapColor.xyz * vec3( 2.0f, 2.0f, 1.0f ) + vec3( -1.0f, -1.0f, 0.0f ) ); // Why this??
	vec3 worldNormal = surfaceToWorld * surfaceNormal;

	vec3 directionToEye = normalize(EYE_POSITION - passWorldPosition);
	
	vec3 surfaceLight = vec3(0.0f);
	vec3 reflectedLight = vec3(0.0f);
	
	// Light Loops
	for	(int i = 0; i < MAX_LIGHTS; ++i)
	{
		// Light Direction
		vec3 surfaceToLightDirection = normalize(LIGHTS[i].m_position - passWorldPosition);
		vec3 directionToLight = mix(surfaceToLightDirection, -LIGHTS[i].m_lightDirection, LIGHTS[i].m_isPointLight);
		
		// Cone Factor
		float cosAngle_LightDir_SurfaceDir = dot(LIGHTS[i].m_lightDirection, -surfaceToLightDirection);
		float coneFactor = smoothstep(LIGHTS[i].m_spotLightOuterCosAngle, LIGHTS[i].m_spotLightInnerCosAngle, cosAngle_LightDir_SurfaceDir);
		
		// Attenuation
		float attenuation = GetAttenuationForLight(i, passWorldPosition);
		
		// Diffuse
		float lightDotNormal = dot(directionToLight, worldNormal);
		lightDotNormal = max(lightDotNormal, 0.0f);
		surfaceLight += (coneFactor * lightDotNormal * LIGHTS[i].m_colorAndIntensity.rgb * attenuation);
		
		// Specular
		vec3 reflectedLightDirection = reflect(-directionToLight, worldNormal);
		float specularFactor = max(0.0f, dot(reflectedLightDirection, directionToEye));
		specularFactor = SPECULAR_AMOUNT * pow(specularFactor, SPECULAR_POWER);
		reflectedLight += (coneFactor * specularFactor * LIGHTS[i].m_colorAndIntensity.rgb * attenuation);
	}
	surfaceLight = clamp(surfaceLight, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f));
   
   
	vec4 finalColor = (vec4(surfaceLight, 1) * surfaceColor) + vec4(reflectedLight, 0);
	outColor = clamp(finalColor, vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
}