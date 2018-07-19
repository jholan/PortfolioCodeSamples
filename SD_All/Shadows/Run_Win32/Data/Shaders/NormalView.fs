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
   // Interpolation is linear, so normals become not normal
   // over a surface, so renormalize it. 
   vec3 surfaceNormal = normalize(passWorldNormal);
   surfaceNormal += 1.0f;
   surfaceNormal /= 2.0f;

   vec4 finalColor = vec4(surfaceNormal, 1.0f);
   
   outColor = finalColor;
}