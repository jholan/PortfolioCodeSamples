#version 420 core


// Textures
layout(binding = 0) uniform sampler2D gTexDiffuse;


// Inputs ----------------------------------------------
in vec4 passColor; 
in vec2 passUV; 
in vec3 passWorldNormal;
in vec3 passWorldPosition;


// Outputs ----------------------------------------------
out vec4 outColor; 



void main( void )
{
   vec4 finalColor = vec4(passUV, 0.0f, 1.0f);
   outColor = finalColor;
}