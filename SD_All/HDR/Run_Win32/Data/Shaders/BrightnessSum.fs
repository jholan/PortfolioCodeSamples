#version 420 core

// Input Textures (color and depth)
layout(binding = 0) uniform sampler2D gTexColor;
layout(binding = 1) uniform sampler2D gTexDepth;
layout(binding = 2) uniform sampler2D gTexBloom;

// Inputs (just UV and Color (fadeout can be done just with color)
in vec2 passUV; 
in vec4 passColor; 

// Output - a single color; 
layout(location = 0) out vec4 outColor;



// Entry Point
void main( void )
{
	vec4 color = texture( gTexColor, passUV );
	vec4 bloomColor = texture(gTexBloom, passUV);
	vec4 bloomedColor = vec4(bloomColor.xyz + color.xyz, 1.0f);
	outColor = bloomedColor;
	return;
}
