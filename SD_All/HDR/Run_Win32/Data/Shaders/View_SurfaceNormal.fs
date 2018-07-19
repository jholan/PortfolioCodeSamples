#version 420 core


// Textures
layout(binding = 0) uniform sampler2D gTexDiffuse;
layout(binding = 1) uniform sampler2D gTexNormal;


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
	vec4 normalMapColor = texture( gTexNormal, passUV );
	vec3 surfaceNormal = normalMapColor.xyz; //normalize( normalMapColor.xyz * vec3( 2.0f, 2.0f, 1.0f ) + vec3( -1.0f, -1.0f, 0.0f ) );
	vec4 finalColor = vec4(surfaceNormal, 1.0f);
   
	outColor = finalColor;
}