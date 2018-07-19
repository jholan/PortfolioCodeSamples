#version 420 core

layout(binding = 0) uniform sampler2D DIFFUSE_SAMPLER;

in vec4 passColor;
in vec2 passUV;
in vec2 passOverlayUV;

out vec4 outColor; 

int NUM_PIXEL_OFFSETS = 4;
ivec2 PIXEL_OFFSETS[] = 
{
	ivec2(-1,0),
	ivec2(1,0),
	ivec2(0,1),
	ivec2(0,-1)
};

float ALPHA_THRESHOLD = 0.5f;
vec4 BORDER_COLOR = vec4(0.0f, 0.0f, 0.0f, 1.0f);

void main( void )
{
	ivec2 textureDim = textureSize(DIFFUSE_SAMPLER, 0);
	float xPixel = clamp(passUV.x * textureDim.x, 0, textureDim.x);
	float yPixel = clamp(passUV.y * textureDim.y, 0, textureDim.y);
	ivec2 pixelCoordinate = ivec2(xPixel, yPixel);
	vec4 pixelColor = texelFetch( DIFFUSE_SAMPLER, pixelCoordinate, 0 ); 
	
	if (pixelColor.a <= ALPHA_THRESHOLD)
	{
		vec4 neighbor_1 = texelFetch( DIFFUSE_SAMPLER, pixelCoordinate + ivec2(-1,0), 0 ); 
		vec4 neighbor_2 = texelFetch( DIFFUSE_SAMPLER, pixelCoordinate + ivec2(1,0),  0 ); 
		vec4 neighbor_3 = texelFetch( DIFFUSE_SAMPLER, pixelCoordinate + ivec2(0,1),  0 ); 
		vec4 neighbor_4 = texelFetch( DIFFUSE_SAMPLER, pixelCoordinate + ivec2(0,-1), 0 ); 
		
		if (neighbor_1.a > ALPHA_THRESHOLD ||
			neighbor_2.a > ALPHA_THRESHOLD ||
			neighbor_3.a > ALPHA_THRESHOLD ||
			neighbor_4.a > ALPHA_THRESHOLD)
		{
			pixelColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
		}
	}
	else
	{
		pixelColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	outColor = pixelColor;
	//outColor = pixelColor * passColor; 
}
