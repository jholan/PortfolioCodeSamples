#version 420 core

// Suggest always manually setting bindings - again, consitancy with 
// other rendering APIs and well as you can make assumptions in your
// engine without having to query
layout(binding = 8) uniform samplerCube gTexSky;

// Inputs
in vec3 passWorldPosition; 
in vec4 passColor; 

// Outputs
out vec4 outColor; 



void main( void )
{
   vec3 sampleDirection = normalize(passWorldPosition); 
   vec4 textureColor = texture( gTexSky, sampleDirection ); 

   outColor = textureColor * passColor;
}
