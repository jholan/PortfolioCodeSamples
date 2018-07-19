#version 420 core

layout(binding=8, std140) uniform MaterialPropertyBlock0
{
	float  	MIP_LEVEL;
	float 	MIP_LEVEL_LCT;
	float 	BLOOM_THRESHOLD;
	float 	PADDING_2;
};


// Input Textures (color and depth)
layout(binding = 0) uniform sampler2D gTexColor;
layout(binding = 1) uniform sampler2D gTexDepth;
layout(binding = 2) uniform sampler2D gTexLuminanceMap;
layout(binding = 3) uniform sampler2D gTexCurrLuminanceMap;

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


// Entry Point
void main( void )
{
	//float l = exp(textureLod(gTexCurrLuminanceMap, passUV, 0).x);
	//outColor = vec4(l, l, l, 1.0f);
	//return;
    //
    //
    //
	//float avgLum = exp(textureLod(gTexLuminanceMap, passUV, MIP_LEVEL).x);
	vec4 color = texture( gTexColor, passUV );
	vec4 bloomColor = texture(gTexCurrLuminanceMap, passUV);
	vec4 bloomedColor = vec4(bloomColor.xyz + color.xyz, 1.0f);
	outColor = bloomedColor;
	return;
	//return;
	////color = ExposeColor(color, avgLum);
	////outColor = color;
	////if (gl_FragCoord.x <= 800)
	////{
	//	//outColor = vec4(Uncharted2(color.xyz), 1.0f);
	//	
	//	// Reinhard
	//	float key = 0.18f;
	//	float avgLuminance = avgLum;
	//	//float scaledLuminance = (key / avgLuminance) * GetLuminance(color.xyz); //color.xyz;
	//	//float tonemappedLuminance = scaledLuminance / (1.0f + scaledLuminance);
	//	//
	//	//float luminanceRatio = tonemappedLuminance / GetLuminance(color.xyz);
	//	//vec3 remappedColor = luminanceRatio * color.xyz;
	//	//outColor = vec4(remappedColor, 1.0f);
	//	
	//	
	//	
	//	vec3 scaledLuminance = (key / avgLuminance) * color.xyz;
	//	outColor = vec4(Reinhard(scaledLuminance), 1.0f);
	////}
	////else
	////{
	//	if (gl_FragCoord.y >= 800 && gl_FragCoord.x >= 1500)
	//	{
	//		float l = textureLod(gTexCurrLuminanceMap, passUV, MIP_LEVEL_LCT).x;
	//		outColor = vec4(l, l, l, 1.0f);
	//	}
	//	//else
	//	//{
	//	//	outColor = vec4(avgLum, avgLum, avgLum, 1.0f);
	//	//}
	//	
	////}
	//return;
	//
	//// Tonemappers
	//outColor = vec4(avgLum, avgLum, avgLum, 1.0f);
	////outColor = textureLod(gTexLuminanceMap, passUV, MIP_LEVEL);
	//return;
	
	
	//outColor = vec4(LinearWithExposure(color.xyz), 1.0f);

	//outColor = vec4(Uncharted2(color.xyz), 1.0f);
	////if (GetLuminance(color.xyz) > 1.0f)
	////{
	////	outColor = vec4(1,0,0,1);
	////}
	//return;
		
	if (gl_FragCoord.x <= 800)
	{
		outColor = vec4(Reinhard(color.xyz), 1.0f);
	}
	else
	{
		vec3 tc = bloomedColor.xyz;
		//if (GetLuminance(color.xyz) >= 1.0f)
		//{
		//	tc = color.xyz;
		//}
		//else
		//{
		//	tc = bloomedColor.xyz;
		//}
		outColor = vec4(Reinhard(tc), 1.0f);
	}
	return;
	
	
	
	if (gl_FragCoord.x <= 533)
	{
		outColor = vec4(Reinhard(color.xyz), 1.0f);
	}
	else if (gl_FragCoord.x <= 1066)
	{
		//outColor = vec4(Uncharted2(color.xyz), 1.0f);
		//outColor = vec4(color.xyz, 1.0f);
		//outColor = vec4(Reinhard(color.xyz), 1.0f);
		outColor = vec4(Uncharted2(color.xyz), 1.0f);
		//outColor = vec4(Filmic(color.xyz), 1.0f);
	}
	else
	{
	//outColor = vec4(Raw(color.xyz), 1.0f);
		outColor = vec4(Filmic(color.xyz), 1.0f);
	}
	return;
   
   
   
	
    vec4 gray = vec4(0.2126, 0.7152, 0.0722, 1);
	float v = dot(color.xyz, vec3(gray));
	//if (v > 1.0f)
	//{
	if (color.r > 1.0f && color.g > 1.0f && color.b > 1.0f)
	{
		outColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	}
	else
	{
		outColor = color;
	}
}
