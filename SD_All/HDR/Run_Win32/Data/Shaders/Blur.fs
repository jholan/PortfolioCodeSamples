#version 420 core


layout(binding=8, std140) uniform MaterialPropertyBlock0
{
	float IS_HORIZONTAL;
};

// Input Textures (color and depth)
layout(binding = 0) uniform sampler2D gTexColor;

uniform float weight[5] = float[] (0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f);


// Inputs (just UV and Color (fadeout can be done just with color)
in vec2 passUV; 
in vec4 passColor; 

// Output - a single color; 
out vec4 outColor; 

// Entry Point
void main( void )
{
	vec2 texelSize = 1.0 / textureSize(gTexColor, 0); // gets size of single texel
    vec3 result = texture(gTexColor, passUV).xyz * weight[0]; // current fragment's contribution
    
	if (IS_HORIZONTAL == 1.0f)
	{
        for(int i = 1; i < 5; ++i)
        {
            result += texture(gTexColor, passUV + vec2(texelSize.x * i, 0.0)).rgb * weight[i];
            result += texture(gTexColor, passUV - vec2(texelSize.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
	{
        for(int i = 1; i < 5; ++i)
        {
            result += texture(gTexColor, passUV + vec2(0.0, texelSize.y * i)).rgb * weight[i];
            result += texture(gTexColor, passUV - vec2(0.0, texelSize.y * i)).rgb * weight[i];
        }
	}
	
    outColor = vec4(result, 1.0);
}
