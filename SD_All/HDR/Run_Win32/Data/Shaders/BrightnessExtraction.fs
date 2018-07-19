#version 420 core

layout(binding=8, std140) uniform MaterialPropertyBlock0
{
	float THRESHOLD;
};

// Input Textures (color and depth)
layout(binding = 0) uniform sampler2D gTexColor;
layout(binding = 1) uniform sampler2D gTexDepth;

// Inputs (just UV and Color (fadeout can be done just with color)
in vec2 passUV; 
in vec4 passColor; 

// Output - a single color; 
out vec4 outColor; 

// this is transposed (vectors are the columns)
mat4 GRAYSCALE_TRANSFORM = { 
   vec4( 0.2126, 0.7152, 0.0722, 0 ),
   vec4( 0.2126, 0.7152, 0.0722, 0 ),
   vec4( 0.2126, 0.7152, 0.0722, 0 ),
   vec4( 0,      0,      0,      1 )
};

// Entry Point
void main( void )
{
   vec4 color = texture( gTexColor, passUV );
   vec4 gray = vec4(0.2126, 0.7152, 0.0722, 1);
   float v = dot(vec3(color), vec3(gray));
   v = max(v, .0001);
   
   if (v >= THRESHOLD)
   {
		outColor = vec4(color.xyz, 1.0f);
   }
   else
   {
		outColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
   }
}