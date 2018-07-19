




#version 420 core

layout(binding=5, std140) uniform ClearUniformBlock 
{
   vec4  CLEAR_COLOR; 
   float DEPTH_TO_CLEAR_TO;
   float SHOULD_CLEAR_DEPTH; // 0 for yes, 1 for no
}; 

// Inputs
in vec3 POSITION;

// Outputs
out vec4 passColor; 



void main( void )
{
	// Outputs
	passColor = CLEAR_COLOR;
	
	vec4 pos = vec4(POSITION, 1.0f);
	pos.z = DEPTH_TO_CLEAR_TO;
	gl_Position = pos;
}
