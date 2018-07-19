#version 420 core

// System Uniform Blocks -------------------------------


// Material Property Blocks ----------------------------
layout (binding=8, std140) uniform MaterialPropertyBlock0
{
	vec4 TINT;
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