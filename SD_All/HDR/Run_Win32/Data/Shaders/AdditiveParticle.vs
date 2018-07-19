#version 420 core


layout(binding=2, std140) uniform CameraUniformBlock 
{
   mat4 VIEW; 
   mat4 PROJECTION; 
}; 

layout(binding=3, std140) uniform ModelUniformBlock 
{
   mat4 MODEL;
}; 

// Inputs
in vec3 POSITION;
in vec4 COLOR;
in vec2 UV; 

// Outputs
out vec4 passColor;
out vec2 passUV;



void main( void )
{
   vec4 localPosition = vec4( POSITION, 1.0f );
   vec4 worldPosition = MODEL * localPosition;
   vec4 clipPosition = PROJECTION * VIEW * worldPosition;

   passUV = UV; 
   passColor = COLOR; 

   gl_Position = clipPosition; // we pass out a clip coordinate
}