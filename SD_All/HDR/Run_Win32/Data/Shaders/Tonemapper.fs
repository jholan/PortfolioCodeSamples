#version 420 core

layout(binding=8, std140) uniform MaterialPropertyBlock0
{
	float  	SPLIT;
	float 	LEFT_TONEMAP;
	float 	RIGHT_TONEMAP;
	float 	PADDING_2;
};


// Input Textures (color and depth)
layout(binding = 0) uniform sampler2D gTexColor;
layout(binding = 1) uniform sampler2D gTexDepth;

// Inputs (just UV and Color (fadeout can be done just with color)
in vec2 passUV; 
in vec4 passColor; 

// Output - a single color; 
layout(location = 0) out vec4 outColor;



vec3 Reinhard(vec3 color)
{
	vec3 reinhard = color.xyz / (color.xyz + vec3(1.0f));
	reinhard = pow(reinhard, vec3(1.0f / 2.2f));
	return reinhard;
}


vec3 RawGammaEncoded(vec3 color)
{
	vec3 raw = pow(color.xyz, vec3(1.0f / 2.2f));
	return raw;
}


vec3 Raw(vec3 color)
{
	return color;
}


vec3 LinearWithExposure(vec3 color)
{
	float exposure = 1.0f;
	vec3 linear = vec3(1.0f) - exp(-color.xyz * exposure);
	linear = pow(linear.xyz, vec3(1.0f / 2.2f));
	return linear;
}


vec3 Filmic(vec3 color)
{
	vec3 x = max(vec3(0),color.xyz-vec3(0.004));
	vec3 filmic = (x*(6.2*x+.5))/(x*(6.2*x+1.7)+0.06);
	return filmic;
}


float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;
float W = 11.2;

vec3 Uncharted2TonemapOperator(vec3 x)
{
   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}


vec3 Uncharted2(vec3 color)
{
	vec3 ExposureBias = vec3(2.0f);
	vec3 curr = Uncharted2TonemapOperator(ExposureBias*color);
	
	vec3 whiteScale = vec3(1.0f)/Uncharted2TonemapOperator(vec3(W));
	vec3 c = curr*whiteScale;
		
	vec3 retColor = pow(c,vec3(1/2.2));
	return retColor;
}


float log10(float value)
{
	return log(value) / log(10.0f);
}

vec4 ExposeColor(vec4 color, float averageLuminance)
{
	float keyValue = 1.03f - (2.0f / (2 + log10(averageLuminance + 1)));
	float linearExposure = (keyValue / averageLuminance);
	float exposure = log2(max(linearExposure, 0.0001f));
	
	vec3 exposedColor = vec3(exp2(exposure)) * color.xyz;
	return vec4(exposedColor, 1.0f);
}


float GetLuminance(vec3 color)
{
	vec3 gray = vec3(0.2126, 0.7152, 0.0722);
	float v = dot(color, gray);
	v = max(v, .0001);
	return v;
}


vec3 TonemapSwitch(vec3 color, float switchOn)
{
	if (switchOn == 0.0f)
	{
		// Raw
		return RawGammaEncoded(color);
	}
	if (switchOn == 1.0f)
	{
		// Reinhard
		return Reinhard(color);
	}
	if (switchOn == 2.0f)
	{
		// Filmic
		return Filmic(color);
	}
	if (switchOn == 3.0f)
	{
		// Uncharted 2
		return Uncharted2(color);
	}

	return color;
}


// Entry Point
void main( void )
{
	vec4 color = texture( gTexColor, passUV );

	if (gl_FragCoord.x <= SPLIT)
	{
		outColor = vec4(TonemapSwitch(color.xyz, LEFT_TONEMAP), 1.0f);
	}
	else
	{
		outColor = vec4(TonemapSwitch(color.xyz, RIGHT_TONEMAP), 1.0f);
	}
	return;
}
