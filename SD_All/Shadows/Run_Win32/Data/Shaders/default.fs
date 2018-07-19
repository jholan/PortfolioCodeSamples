#version 420 core

layout(binding = 0) uniform sampler2D DIFFUSE_SAMPLER;

in vec4 passColor;
in vec2 passUV;
in vec2 passOverlayUV;

out vec4 outColor; 


void main( void )
{
	vec4 diffuse = texture(DIFFUSE_SAMPLER, passUV);
	outColor = diffuse * passColor; 
}
