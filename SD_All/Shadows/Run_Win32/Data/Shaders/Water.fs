#version 420 core

// System Uniform Blocks -------------------------------
layout(binding=1, std140) uniform TimeUniformBlock
{
	float		SYSTEM_LIFETIME;
	float		GAME_LIFETIME;
	float		SYSTEM_DELTA_SECONDS;
	float		GAME_DELTA_SECONDS;
};

// Material Property Blocks ----------------------------
layout (binding=8, std140) uniform MaterialPropertyBlock0
{
	vec4 	TINT;
	
	float	SCROLL_SPEED;
	float	MPB_PADDING_0;
	float	MPB_PADDING_1;
	float	MPB_PADDING_2;
};


// Textures --------------------------------------------
layout(binding = 0) uniform sampler2D DIFFUSE_SAMPLER;


// Inputs ----------------------------------------------
in vec4 passColor;
in vec2 passUV; 


// Outputs ---------------------------------------------
out vec4 outColor; 



void main( void )
{
	vec4 color = texture( DIFFUSE_SAMPLER, passUV ) * TINT * passColor; 

	vec4 finalColor = color;
	outColor = clamp(finalColor, vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
}