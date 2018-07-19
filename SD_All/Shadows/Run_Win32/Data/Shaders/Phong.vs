#version 420 core


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

// Inputs
in vec3 POSITION;
in vec4 COLOR;
in vec3 NORMAL;
in vec2 UV; 
in vec4 TANGENT;

// Outputs
out vec4 passColor;
out vec2 passUV;
out vec3 passWorldNormal;
out vec3 passWorldPosition;
out vec3 passWorldTangent;
out vec3 passWorldBitangent;
out vec3 passCameraPosition;



void main( void )
{
   vec4 localPosition = vec4( POSITION, 1.0f );
   vec4 worldPosition = MODEL * localPosition;
   vec4 viewPosition = VIEW * worldPosition;
   vec4 clipPosition = PROJECTION * viewPosition;
   

   passUV = UV; 
   passColor = COLOR; 
   
   passWorldPosition = worldPosition.xyz;
   
   passWorldNormal = normalize( (MODEL * vec4(NORMAL, 0.0f)).xyz ); 
   passWorldTangent = normalize( (MODEL * vec4(TANGENT.xyz, 0.0f)).xyz ); 
   passWorldBitangent = normalize( TANGENT.w * cross(passWorldTangent, passWorldNormal) );
   
   passCameraPosition = viewPosition.xyz;

   gl_Position = clipPosition; // we pass out a clip coordinate
}