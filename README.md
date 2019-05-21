# PortfolioCodeSamples


This is a handfull of C++ and HLSL files from my current working engine.

HLSL files use the following format
.vs - Vertex Shader
.fs - Fragment/ Pixel Shader
.cs - Compute Shader
.inc - A library file, this is meant to be included in another shader

HLSL file breakdown
Punctual PBR Lighting
	Lighting.inc
	BRDF.inc
	Fresnel.inc
	Geometry.inc
	NDF.inc
	Attenuation.inc
	
Shadows 
	SpotLight.fs - A shadow casting spot light, also has test code for percentage closer soft shadows

Tiled Deferred Renderer
	LightListGenerator.cs - A compute shader that buckets the lights per screen tile for later use in shading

Shader Setup
	Shaders.xml - Contains shader definitions 
	
Vertex Shader
	Passthrough_PCUTBN.vs - A simple passthrough vertex shader for a vertex containing position, color, uvs, normal, tangent, and bitangent