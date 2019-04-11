
#include "Constants.inc"
#include "Lighting.inc"
#include "SystemConstantBuffers.inc"
 
#define PCF_NUM_SAMPLES 64

// PCSS still in development we should be calculating these from light data
#define BLOCKER_SEARCH_NUM_SAMPLES 16
#define LIGHT_NEAR_PLANE 1.0
#define LIGHT_WORLD_SIZE .1
#define LIGHT_FRUSTUM_WIDTH 127.6
#define LIGHT_SIZE_UV (LIGHT_WORLD_SIZE / LIGHT_FRUSTUM_WIDTH)


// User Constant Buffers ----------------------------------------
cbuffer LightConstantBuffer : register(b7)
{
	SpotLightData LIGHT;
}



// Textures -----------------------------------------------------
Texture2D 	GBUFFER_0 					: register(t0);
Texture2D 	GBUFFER_1 					: register(t1);
Texture2D 	GBUFFER_2					: register(t2);
Texture2D 	GBUFFER_3 					: register(t3);
Texture2D	SHADOW_MAP					: register(t4);



// Samplers ----------------------------------------------------
SamplerState 			LINEAR_SAMPLER 			: register(s0);
SamplerComparisonState  SHADOW_SAMPLER          : register(s1);
SamplerState			POINT_SAMPLER			: register(s2);



// Inputs/ Outputs ----------------------------------------------
struct FragmentInputs
{
	float4 position 	: SV_POSITION;
	float4 color		: COLOR;
	float2 uvs 			: UV;
};



// Functions ----------------------------------------------------
SurfaceProperties UnpackGBuffer(float2 uvs)
{
	SurfaceProperties surface;

	
	// Texture reads
	float4 gbuffer0Value = GBUFFER_0.Sample(LINEAR_SAMPLER, uvs); 
	float4 gbuffer1Value = GBUFFER_1.Sample(LINEAR_SAMPLER, uvs);
	float4 gbuffer2Value = GBUFFER_2.Sample(LINEAR_SAMPLER, uvs);
	float4 gbuffer3Value = GBUFFER_3.Sample(LINEAR_SAMPLER, uvs);	
	
	// Material Properties
	// 	-> From Textures
	surface.albedo 	 	= gbuffer0Value.xyz;
	surface.metalness	= gbuffer2Value.g;
	surface.roughness 	= clamp(gbuffer2Value.r, MIN_ROUGHNESS, MAX_ROUGHNESS);
	surface.normal_WS 	= gbuffer1Value.xyz;
	surface.position_WS = gbuffer3Value.xyz;
	
	// Material Reflectance
	float3 baseDielectricF0 = float3(0.04f, 0.04f, 0.04f);						// Default dielectric F0
	surface.F0 = lerp(baseDielectricF0, surface.albedo, surface.metalness); 	// If we are a metal use the albedo as the F0 value otherwise use the above F0
	
	
	return surface;
}


float CalculatePCFShadowCoverage(Texture2D shadowMap, SamplerComparisonState pcfSampler, float2 uv, float receiverDepth, int kernelSize)
{
	// Get shadow map size and uv offsets
	float2 shadowMapDimensions;
	shadowMap.GetDimensions(shadowMapDimensions.x, shadowMapDimensions.y);
	float2 shadowMapTexelSizeInUVs = 1.0f / (2.0f * shadowMapDimensions);
	
	
	// Calc kernel size and pixel offsets
	int minOffset = -(int)((kernelSize - 1) * 0.5f);
	int maxOffset =  (int)((kernelSize - 1) * 0.5f);
	kernelSize = (maxOffset - minOffset) + 1;
	
	// Average the shadow shadow factors
	float shadowFactor = 0.0f;
	[unroll]
	for (int y = minOffset; y <= maxOffset; ++y)
	{
		for (int x = minOffset; x <= maxOffset; ++x)
		{	
			float2 sampleUVs = uv + (shadowMapTexelSizeInUVs * float2(x, y));
			float shadowMapCoverage = shadowMap.SampleCmp(pcfSampler, sampleUVs, receiverDepth);
			shadowFactor += shadowMapCoverage;
		}
	}
	shadowFactor /= (float)(kernelSize * kernelSize);
	
	return shadowFactor;
}
	
float CalculatePenumbraSize(float receiverDepth, float blockerDepth) //Parallel plane estimation
{
	float penumbraSize = (receiverDepth - blockerDepth) / blockerDepth;
	return penumbraSize;
}
	
static const float2 poissonDiskRandomValues[64] = 
{
	float2(0.0617981, 0.07294159),
	float2(0.6470215, 0.7474022),
	float2(-0.5987766, -0.7512833),
	float2(-0.693034, 0.6913887),
	float2(0.6987045, -0.6843052),
	float2(-0.9402866, 0.04474335),
	float2(0.8934509, 0.07369385),
	float2(0.1592735, -0.9686295),
	float2(-0.05664673, 0.995282),
	float2(-0.1203411, -0.1301079),
	float2(0.1741608, -0.1682285),
	float2(-0.09369049, 0.3196758),
	float2(0.185363, 0.3213367),
	float2(-0.1493771, -0.3147511),
	float2(0.4452095, 0.2580113),
	float2(-0.1080467, -0.5329178),
	float2(0.1604507, 0.5460774),
	float2(-0.4037193, -0.2611179),
	float2(0.5947998, -0.2146744),
	float2(0.3276062, 0.9244621),
	float2(-0.6518704, -0.2503952),
	float2(-0.3580975, 0.2806469),
	float2(0.8587891, 0.4838005),
	float2(-0.1596546, -0.8791054),
	float2(-0.3096867, 0.5588146),
	float2(-0.5128918, 0.1448544),
	float2(0.8581337, -0.424046),
	float2(0.1562584, -0.5610626),
	float2(-0.7647934, 0.2709858),
	float2(-0.3090832, 0.9020988),
	float2(0.3935608, 0.4609676),
	float2(0.3929337, -0.5010948),
	float2(-0.8682281, -0.1990303),
	float2(-0.01973724, 0.6478714),
	float2(-0.3897587, -0.4665619),
	float2(-0.7416366, -0.4377831),
	float2(-0.5523247, 0.4272514),
	float2(-0.5325066, 0.8410385),
	float2(0.3085465, -0.7842533),
	float2(0.8400612, -0.200119),
	float2(0.6632416, 0.3067062),
	float2(-0.4462856, -0.04265022),
	float2(0.06892014, 0.812484),
	float2(0.5149567, -0.7502338),
	float2(0.6464897, -0.4666451),
	float2(-0.159861, 0.1038342),
	float2(0.6455986, 0.04419327),
	float2(-0.7445076, 0.5035095),
	float2(0.9430245, 0.3139912),
	float2(0.0349884, -0.7968109),
	float2(-0.9517487, 0.2963554),
	float2(-0.7304786, -0.01006928),
	float2(-0.5862702, -0.5531025),
	float2(0.3029106, 0.09497032),
	float2(0.09025345, -0.3503742),
	float2(0.4356628, -0.0710125),
	float2(0.4112572, 0.7500054),
	float2(0.3401214, -0.3047142),
	float2(-0.2192158, -0.6911137),
	float2(-0.4676369, 0.6570358),
	float2(0.6295372, 0.5629555),
	float2(0.1253822, 0.9892166),
	float2(-0.1154335, 0.8248222),
	float2(-0.4230408, -0.7129914)
};									 

void FindBlocker(out float avgBlockerDepth, out float numBlockedSamples, Texture2D shadowMap, SamplerState pointSampler, float2 uv, float receiverDepth)
{
	//This uses similar triangles to compute what
	//area of the shadow map we should search
	float searchWidthInUVs = LIGHT_SIZE_UV * (receiverDepth - LIGHT_NEAR_PLANE) / receiverDepth;
	float blockerDepthSum = 0;
	numBlockedSamples = 0;
	
	for(int i = 0; i < BLOCKER_SEARCH_NUM_SAMPLES; ++i)
	{
		float shadowMapDepth = shadowMap.SampleLevel(pointSampler, uv + poissonDiskRandomValues[i] * searchWidthInUVs, 0);
		if ( shadowMapDepth < receiverDepth ) 
		{
			blockerDepthSum += shadowMapDepth;
			numBlockedSamples++;
		}
	}
	
	avgBlockerDepth = blockerDepthSum / numBlockedSamples;
} 

float CalculatePCFShadowCoverage_PoissonDisk(Texture2D shadowMap, SamplerComparisonState shadowSampler, float2 uv, float receiverDepth, float filterRadiusInUVs)
{
	float sum = 0.0f;
	for ( int i = 0; i < PCF_NUM_SAMPLES; ++i )
	{
		float2 offset = poissonDiskRandomValues[i] * filterRadiusInUVs;
		sum += shadowMap.SampleCmpLevelZero(shadowSampler, uv + offset, receiverDepth);
	}
	
	float shadowFactor = sum / (PCF_NUM_SAMPLES);
	return shadowFactor;
} 				  
									  
// receiver depth is assumed to be in view space
float CalculatePCSSShadowCoverage(Texture2D shadowMap, SamplerState pointSampler, SamplerComparisonState pcfSampler, float2 uv, float receiverDepth)
{	
	// STEP 1: blocker search
	float avgBlockerDepth = 0;
	float numBlockedSamples = 0;
	FindBlocker(avgBlockerDepth, numBlockedSamples, shadowMap, pointSampler, uv, receiverDepth);
	
	// SHORT CIRCUIT
	if(numBlockedSamples < 1.0)
	{
		// Not in shadow
		return 1.0f;
	}
	
	
	// STEP 2: penumbra size
	float penumbraRatio = CalculatePenumbraSize(receiverDepth, avgBlockerDepth);
	float filterRadiusUV = penumbraRatio * LIGHT_SIZE_UV * LIGHT_NEAR_PLANE / receiverDepth;
	
	// STEP 3: filter
	float shadowFactor = CalculatePCFShadowCoverage_PoissonDisk(shadowMap, pcfSampler, uv, receiverDepth, filterRadiusUV);
	return shadowFactor; 
}									  
									  
									  



// Entry Point --------------------------------------------------
float4 Main_FragmentShader(FragmentInputs input) : SV_TARGET
{
	float4 outColor = float4(-1,-1,-1,1);

	SurfaceProperties surface = UnpackGBuffer(input.uvs);
	
	
	// Shadows -------------------------------------------------------------------------------------------------------------------------
	// Get the uv coords to sample the shadow map, and the depth to compare to
	float4 lightSpacePosition = mul(LIGHT.m_shadowViewProjection, float4(surface.position_WS, 1.0f));
	float3 lightNDCPosition = lightSpacePosition.xyz / lightSpacePosition.w; 						// Manual perspective devide
	float3 uv_and_depth = float3(
									(lightNDCPosition.x * 0.5f) + 0.5f,
									1.0f - ((lightNDCPosition.y * 0.5f) + 0.5f),
									lightNDCPosition.z
								);
	
	// Get the depth from the shadow map
	float myDepth = uv_and_depth.z - 0.006f; // Bias, this could be pulled out into a user tweakable value
	myDepth = min(myDepth, 1.0f);
	
	// PCF
	float shadowFactor = 0.0f;
	//shadowFactor = CalculatePCFShadowCoverage(SHADOW_MAP, SHADOW_SAMPLER, uv_and_depth.xy, myDepth, 8);
	shadowFactor = CalculatePCFShadowCoverage_PoissonDisk(SHADOW_MAP, SHADOW_SAMPLER, uv_and_depth.xy, myDepth, .002);
	//shadowFactor = CalculatePCSSShadowCoverage(SHADOW_MAP, POINT_SAMPLER, SHADOW_SAMPLER, uv_and_depth.xy, myDepth); 
	
	
	// Lighting --------------------------------------------------------------------------------------------------------------------------
	// Calculate outgoing Luminance(L_o)
	float3 luminanceOut = EvaluateSpotLight(surface, LIGHT, EYE_POSITION) * shadowFactor;
	
	
	outColor = float4(luminanceOut, 1.0f);
	return outColor;
};
