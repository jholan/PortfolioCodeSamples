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

// Outputs
out vec3 passWorldPosition;



void main( void )
{
	// 0, since I don't want to translate
	vec4 localPosition = vec4( POSITION, 0.0f );	
	
	vec4 worldPosition = localPosition; // assume local is world for now; 
    vec4 cameraPosition = VIEW * localPosition;
	vec4 clipPosition = PROJECTION * vec4(cameraPosition.xyz, 1.0f); // projection relies on a 1 being present, so add it back

	
	
	// Outputs
	passWorldPosition = POSITION; //worldPosition.xyz;	
	
	// we only render where depth is 1.0 (equal, ie, where have we not drawn)
	// so z needs to be one for all these
	clipPosition.z = clipPosition.w; // z must equal w.  We set the z, not the w, so we don't affect the x and y as well
	gl_Position = clipPosition; //PROJECTION * VIEW * vec4(POSITION, 1.0f);//
}
