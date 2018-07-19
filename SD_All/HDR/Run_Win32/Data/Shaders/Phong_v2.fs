#version 420 core

// Defines -----------------------------------------------
#define MAX_LIGHTS 8
#define FOG
#define NUM_CASCADES 3



// Structs -----------------------------------------------
struct LightData
{
	vec4  m_colorAndIntensity; 	// rgb color, a intensity
	
	vec3  m_lightDirection;
	float m_isPointLight; 		// 0 for yes, 1 for no
	
	vec3  m_position;
	float m_spotLightInnerCosAngle;
	
	vec3  m_attenuation;	
	float m_spotLightOuterCosAngle;  
};

struct ShadowMapData
{
	mat4  m_viewProjection;
};

struct CascadeBreakpoint
{
	float m_breakpoint;
	float m_cb_padding_0;
	float m_cb_padding_1;
	float m_cb_padding_2;
};



// Uniforms ----------------------------------------------
// System Uniform Blocks ---------------------------------
layout(binding=2, std140) uniform CameraUniformBlock 
{
   mat4 				VIEW; 
   mat4 				PROJECTION; 
			
   vec3 				EYE_POSITION;
   float 				PADDING_0;
}; 

layout(binding=4, std140) uniform LightUniformBlock 
{
	vec4 	  			AMBIENT;

	LightData			SUN;
	ShadowMapData   	SUN_CASCADES[NUM_CASCADES];

	CascadeBreakpoint 	CASCADE_BREAKPOINTS[NUM_CASCADES];
	
	LightData 			LIGHTS[MAX_LIGHTS];
	ShadowMapData 		SHADOWMAPS[MAX_LIGHTS];
}; 

layout(binding=6, std140) uniform FogUniformBlock
{
	vec4 				FOG_COLOR;
				
	float 				FOG_NEAR_PLANE;		// What depth to start the fog at
	float				FOG_FAR_PLANE;		// What depth to end the fog at
	float				FOG_NEAR_FACTOR;	// Amount of fog at the near plane
	float				FOG_FAR_FACTOR; 	// Amount of fog at the far plane
};


// User Property Blocks (Shader Replicated) --------------
layout(binding=8, std140) uniform MaterialPropertyBlock0
{
	float 				SPECULAR_AMOUNT;
	float 				SPECULAR_POWER;
};



// Textures ----------------------------------------------
layout(binding = 0)  uniform sampler2D gTexDiffuse;
layout(binding = 1)  uniform sampler2D gTexNormal; 
layout(binding = 2)  uniform sampler2D gTexShadowMaps[8];
layout(binding = 10) uniform sampler2D gTexSunCascades[NUM_CASCADES];



// Inputs ------------------------------------------------
in vec4 passColor;
in vec2 passUV; 
in vec3 passWorldNormal;
in vec3 passWorldPosition;
in vec3 passWorldTangent;
in vec3 passWorldBitangent;
in vec3 passCameraPosition;



// Outputs -----------------------------------------------
layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outColor_Bloom;



// Functions ---------------------------------------------
// Lighting ----------------------------------------------
float GetAttenuationForLight(in int index, in vec3 surfacePoint)
{
	float  dist = length(LIGHTS[index].m_position - surfacePoint);
	vec3   distances = vec3(1.0f, dist, dist * dist);
	float  attenuation = LIGHTS[index].m_colorAndIntensity.a / dot(LIGHTS[index].m_attenuation, distances);
	//attenuation = clamp(attenuation, 0.0f, 1.0f);
	attenuation = max(attenuation, 0.0f);
	return attenuation;
}

// Fog ---------------------------------------------------
vec4 AddFog(vec4 color, float viewDepth)
{
	#if defined(FOG)
		float fogFactor = smoothstep(FOG_NEAR_PLANE, FOG_FAR_PLANE, viewDepth);
		fogFactor = FOG_NEAR_FACTOR + (FOG_FAR_FACTOR - FOG_NEAR_FACTOR) * fogFactor;
		color = mix(color, FOG_COLOR, fogFactor);
	#endif
	
	return color;
}

// Luminance ---------------------------------------------
float GetLuminance(vec3 color)
{
	vec3 gray = vec3(0.2126, 0.7152, 0.0722);
	float v = dot(color, gray);
	v = max(v, .0001);
	return v;
}


// Main --------------------------------------------------
void main( void )
{
	// Texture reads
	vec4 surfaceColor = texture( gTexDiffuse, passUV ); 
	vec4 normalMapColor = texture( gTexNormal, passUV );
	
	
	
	// Rendering values
	vec3 directionToEye = normalize(EYE_POSITION - passWorldPosition);

	
	// Normal
	// TBN Space
	vec3 vertexNormal = normalize(passWorldNormal);   	// Renormaize because of interpolation
	vec3 vertexTangent = normalize(passWorldTangent);
	vec3 vertexBitangent = normalize(passWorldBitangent);
	mat3 surfaceToWorld = mat3(vertexTangent, vertexBitangent, vertexNormal);
	
	// Get the world normal 
	vec3 surfaceNormal = normalize( normalMapColor.xyz * vec3( 2.0f, 2.0f, 1.0f ) + vec3( -1.0f, -1.0f, 0.0f ) ); // Why this??
	vec3 worldNormal = surfaceToWorld * surfaceNormal;
	worldNormal = vertexNormal; 
	
	
	
	// Lighting
	// Ambient
	vec3 surfaceLight = vec3(0.0f);
	vec3 reflectedLight = vec3(0.0f);
	surfaceLight = AMBIENT.rgb * AMBIENT.a;
	
	
	
	// Sun
	// Determine the correct cascade to use based on our depth in camera space
	int cascadeToQuery = NUM_CASCADES - 1;
	for (int i = 0; i < NUM_CASCADES; ++i)
	{
		if (passCameraPosition.z < CASCADE_BREAKPOINTS[i].m_breakpoint)
		{
			cascadeToQuery = i;
			break;
		}
	}
	
	// Shadows -------------------------------------------------------------------------------------------------------------------------
	// Get the uv coords to sample the shadow map, and the depth to compare to
	vec4 sunSpacePosition = SUN_CASCADES[cascadeToQuery].m_viewProjection * vec4(passWorldPosition, 1.0f);
	vec3 sunNDCPosition = sunSpacePosition.xyz / sunSpacePosition.w; 						// Manual perspective devide
	vec3 sun_uv_and_depth = (sunNDCPosition * 0.5f) + 0.5f;
	
	// Get the depth from the shadow map
	float sun_shadowMapDepth = texture(gTexSunCascades[cascadeToQuery], sun_uv_and_depth.xy).r;
	float sun_shadowFactor = (sun_uv_and_depth.z - 0.0005f > sun_shadowMapDepth ? 0.0f : 1.0f );
	
	// Lighting ------------------------------------------------------------------------------------------------------------------------
	// Light Direction
	vec3 sun_directionToLight = -SUN.m_lightDirection;
	
	// Attenuation
	float sun_attenuation = clamp(SUN.m_colorAndIntensity.a, 0.0f, 1.0f);
	
	// Diffuse
	float sun_lightDotNormal = dot(sun_directionToLight, worldNormal);
	float sun_specClamp = step(0, sun_lightDotNormal);
	sun_lightDotNormal = max(sun_lightDotNormal, 0.0f);
	surfaceLight += (sun_shadowFactor * (sun_lightDotNormal * SUN.m_colorAndIntensity.rgb * sun_attenuation));
	
	// Specular
	vec3 sun_reflectedLightDirection = reflect(-sun_directionToLight, worldNormal);
	float sun_specularFactor = max(0.0f, dot(sun_reflectedLightDirection, directionToEye));
	sun_specularFactor = sun_specClamp * SPECULAR_AMOUNT * pow(sun_specularFactor, SPECULAR_POWER);
	reflectedLight += (sun_shadowFactor * (sun_specularFactor * SUN.m_colorAndIntensity.rgb * sun_attenuation));
	
	
	
	// Punctual Light Loops
	for	(int i = 0; i < MAX_LIGHTS; ++i)
	{
		// Shadows -------------------------------------------------------------------------------------------------------------------------
		// Get the uv coords to sample the shadow map, and the depth to compare to
		vec4 lightSpacePosition = SHADOWMAPS[i].m_viewProjection * vec4(passWorldPosition, 1.0f);
		vec3 lightNDCPosition = lightSpacePosition.xyz / lightSpacePosition.w; 						// Manual perspective devide
		vec3 uv_and_depth = (lightNDCPosition * 0.5f) + 0.5f;
		
		// Get the depth from the shadow map
		float shadowMapDepth = texture(gTexShadowMaps[i], uv_and_depth.xy).r;
		float myDepth = uv_and_depth.z - 0.0005f;
		myDepth = min(myDepth, 1.0f);
		float shadowFactor = (myDepth > shadowMapDepth ? 0.0f : 1.0f );
		
		
		// Lighting ------------------------------------------------------------------------------------------------------------------------
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
		float specClamp = step(0, lightDotNormal);
		lightDotNormal = max(lightDotNormal, 0.0f);
		surfaceLight += (shadowFactor * (coneFactor * lightDotNormal * LIGHTS[i].m_colorAndIntensity.rgb * attenuation));

		// Specular
		vec3 reflectedLightDirection = reflect(-directionToLight, worldNormal);
		float specularFactor = max(0.0f, dot(reflectedLightDirection, directionToEye));
		specularFactor = specClamp * SPECULAR_AMOUNT * pow(specularFactor, SPECULAR_POWER);
		reflectedLight += (shadowFactor * (coneFactor * specularFactor * LIGHTS[i].m_colorAndIntensity.rgb * attenuation));
	}
	//surfaceLight = clamp(surfaceLight, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f));
	vec4 slTotal = vec4(surfaceLight, 1) * surfaceColor;
	//slTotal = clamp(slTotal, vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	vec4 finalColor = slTotal + vec4(reflectedLight, 0);
	//vec4 finalColor = (vec4(surfaceLight, 1) * surfaceColor) + vec4(reflectedLight, 0);
	
	
	
	//if (passCameraPosition.z <= CASCADE_BREAKPOINTS[0].m_breakpoint)
	//{
	//		finalColor = finalColor + vec4(0.0f, 0.0f, .5f, 1.0f);
	//}
	//else if (passCameraPosition.z <= CASCADE_BREAKPOINTS[1].m_breakpoint)
	//{
	//		finalColor = finalColor + vec4(0.0f, .5f, 0.0f, 1.0f);
	//}
	//else if (passCameraPosition.z <= CASCADE_BREAKPOINTS[2].m_breakpoint)
	//{
	//		finalColor = finalColor + vec4(.5f, 0.0f, 0.0f, 1.0f);
	//}
	
	outColor = finalColor;
	outColor_Bloom = vec4(vec3(GetLuminance(outColor.xyz)), 1.0f);
	//outColor = clamp(finalColor, vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	outColor = AddFog(outColor, passCameraPosition.z);
}