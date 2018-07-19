#version 420 core

layout(binding=8, std140) uniform MaterialPropertyBlock0
{
	vec4  END_COLOR;

	float PERCENT_COMPLETE;
	float PADDING_0;
	float PADDING_1;
	float PADDING_2;
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
   vec4 color = texture( gTexColor, passUV );
   //vec4 gray = vec4(0.2126, 0.7152, 0.0722, 1);
   //float v = dot(vec3(color), vec3(gray));
   //vec4 grayscaleColor = vec4(vec3(v), 1);
   
   outColor = mix(color, END_COLOR, PERCENT_COMPLETE);  
   //outColor = vec4(255,0,0,1);
}
