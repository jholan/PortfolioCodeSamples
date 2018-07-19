#version 420 core

layout(binding=2, std140) uniform CameraUniformBlock 
{
   mat4 VIEW; 
   mat4 PROJECTION; 
}; 

//uniform mat4 PROJECTION;
//uniform mat4 VIEW;

in vec3 POSITION;
in vec4 COLOR;
in vec2 UV;

out vec4 passColor;
out vec2 passUV;

void main( void )
{
	// Position
	vec4 localPosition = vec4( POSITION, 1 );
   
	vec4 clipPosition = PROJECTION * VIEW * localPosition;
   
	gl_Position = clipPosition;
   
   
	// UVs
	passUV = UV;
  
   
	// Color
	passColor = COLOR;
}
