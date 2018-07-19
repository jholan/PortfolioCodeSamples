#version 420 core

// System Uniform Blocks -------------------------------
layout(binding=2, std140) uniform CameraUniformBlock 
{
   mat4 	VIEW; 
   mat4 	PROJECTION; 
   
   vec3 	EYE_POSITION;
   float 	PADDING_0;
}; 

layout(binding=3, std140) uniform ModelUniformBlock 
{
   mat4 MODEL;
}; 


// Material Property Blocks ----------------------------


// Inputs ----------------------------------------------
in vec3 POSITION;

// Outputs ---------------------------------------------

void main()
{
	vec4 localPosition = vec4( POSITION, 1.0f );
	vec4 worldPosition = MODEL * localPosition;
	vec4 clipPosition = PROJECTION * VIEW * worldPosition;
   
	gl_Position = clipPosition; // we pass out a clip coordinate
}