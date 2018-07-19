
// Hellp





#version 420 core

// Textures
layout(binding = 0) uniform sampler2D gTexDiffuse;


// Inputs ----------------------------------------------
in vec4 passColor; 
in vec2 passUV; 


// Outputs ----------------------------------------------
out vec4 outColor; 



void main( void )
{
	vec4 finalColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);

	#ifdef TEXTURES
	vec4 surfaceColor = texture( gTexDiffuse, passUV ); 
	finalColor = surfaceColor;
	#endif
	
	outColor = finalColor;
}