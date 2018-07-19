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
	vec4 surfaceColor = texture( gTexDiffuse, passUV ); 
	vec4 normalMapColor = texture( gTexNormal, passUV );
    
	vec3 vertexNormal = normalize(passWorldNormal);   	// Renormaize because of interpolation
	vec3 vertexTangent = normalize(passWorldTangent);
	vec3 vertexBitangent = normalize(passWorldBitangent);
	mat3 surfaceToWorld = mat3(vertexTangent, vertexBitangent, vertexNormal);
	
	vec3 surfaceNormal = normalize( normalMapColor.xyz * vec3( 2.0f, 2.0f, 1.0f ) + vec3( -1.0f, -1.0f, 0.0f ) );
	vec3 worldNormal = normalize(surfaceToWorld * surfaceNormal);
	worldNormal = (worldNormal + vec3(1.0f)) * .5f; 
	
	vec4 finalColor = vec4(worldNormal, 1.0f);
	outColor = finalColor;
}