#version 420 core

layout(binding=8, std140) uniform MaterialPropertyBlock0
{
	float  	oneOverGamma;
	float 	PADDING_0;
	float 	PADDING_1;
	float 	PADDING_2;
};

// Input Textures (color and depth)
layout(binding = 0) uniform sampler2D gTexColor;
layout(binding = 1) uniform sampler2D gTexDepth;

// Inputs (just UV and Color (fadeout can be done just with color)
in vec2 passUV; 
in vec4 passColor; 

// Output - a single color; 
out vec4 outColor; 



// Entry Point
void main( void )
{
	float oOG = 1.0f / 2.2f;

   vec4 color = texture( gTexColor, passUV );   
   vec3 gammaEncodedColor = pow(color.xyz, vec3(oOG));
   
   
   
   outColor = vec4(gammaEncodedColor, 1.0f);
   //outColor = color;
}
