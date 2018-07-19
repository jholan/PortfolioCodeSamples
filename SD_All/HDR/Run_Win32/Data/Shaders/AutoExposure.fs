#version 420 core

// System Uniform Blocks -------------------------------
layout(binding=1, std140) uniform TimeUniformBlock
{
	float		SYSTEM_LIFETIME;
	float		GAME_LIFETIME;
	float		SYSTEM_DELTA_SECONDS;
	float		GAME_DELTA_SECONDS;
};

layout(binding=8, std140) uniform MaterialPropertyBlock0
{
	float EXPOSURE_SPEED;
	float TARGET_EXPOSURE;
	
	float MIP_LEVEL_PREVIOUS;
	float MIP_LEVEL_CURRENT;
};

// Input Textures (color and depth)
layout(binding = 0) uniform sampler2D gTexPreviousLuminanace;
layout(binding = 1) uniform sampler2D gTexCurrentLuminance;

// Inputs (just UV and Color (fadeout can be done just with color)
in vec2 passUV; 
in vec4 passColor; 

// Output - a single color; 
out vec4 outColor; 


// Entry Point
void main( void )
{

	float previousLuminance = exp(textureLod(gTexPreviousLuminanace, passUV, MIP_LEVEL_PREVIOUS).x);
	float currentLuminance = textureLod(gTexCurrentLuminance, passUV, MIP_LEVEL_CURRENT).x;
	
	
	float adaptedLuminance = previousLuminance + (currentLuminance - previousLuminance) * (1.0f - exp(-GAME_DELTA_SECONDS * EXPOSURE_SPEED));
	
	adaptedLuminance = log(adaptedLuminance);
	outColor = vec4(adaptedLuminance, adaptedLuminance, adaptedLuminance, 1.0f);
	return;
}
