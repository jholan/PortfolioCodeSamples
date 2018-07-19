#version 420 core


// Textures
layout(binding = 0) uniform sampler2D gTexDiffuse;


// Inputs ----------------------------------------------
in vec4 passColor;
in vec2 passUV; 
in vec3 passWorldNormal;
in vec3 passWorldPosition;
in vec3 passWorldTangent;
in vec3 passWorldBitangent;


// Outputs ----------------------------------------------
out vec4 outColor; 



void main( void )
{
   // Interpolation is linear, so normals become not normal
   // over a surface, so renormalize it. 
   vec3 surfaceBitangent = normalize(passWorldBitangent);
   surfaceBitangent += 1.0f;
   surfaceBitangent /= 2.0f;

   vec4 finalColor = vec4(surfaceBitangent, 1.0f);
   
   outColor = finalColor;
}