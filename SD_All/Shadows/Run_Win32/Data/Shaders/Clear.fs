#version 420 core

// Inputs
in vec4 passColor; 

// Outputs
out vec4 outColor; 



void main( void )
{
	
	outColor = pow(passColor, vec4(1.0f / 2.2f));
}
