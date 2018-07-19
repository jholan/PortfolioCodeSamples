#version 420 core

layout(binding = 0) uniform sampler2D gTexEquirectangularMap;

// Inputs
in vec3 passWorldPosition; 

// Outputs
out vec4 outColor; 

const float PI = 3.1415926535897932384626433832795f;
const float ONE_OVER_2_PI = 1.0f / (2.0f * PI);
const float ONE_OVER_PI = 1.0f / PI;


vec2 GetUVsFromUVSphere(vec3 sampleDirection)
{
	vec2 uv = vec2(atan(-sampleDirection.z, sampleDirection.x), asin(sampleDirection.y));
	uv.x *= ONE_OVER_2_PI;
	uv.y *= ONE_OVER_PI;
	uv.y += 0.5f;
	
	if (uv.x < 0.0f)
	{
		uv.x = 1.0f + uv.x;
	}
	
	if (uv.y < 0.0f)
	{
		uv.y = 1.0f + uv.y;
	}
	
	return uv;
}



void main( void )
{
   vec2 equirectangularUVs = GetUVsFromUVSphere(normalize(passWorldPosition));
   vec3 color = texture(gTexEquirectangularMap, equirectangularUVs).rgb;

   outColor = vec4(color, 1.0f);
}
