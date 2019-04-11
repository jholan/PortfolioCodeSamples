/* 
	Shader Key
	_LS -> Local Space
	_WS -> World Space
	_VS -> View Space
	_CS -> Clip Space
*/
#include "SystemConstantBuffers.inc"



// Inputs/ Outputs ----------------------------------------------
struct VertexInputs
{
	float3 position : POSITION;
	float4 color 	: COLOR;
	float3 normal 	: NORMAL;
	float2 uvs 		: UV;
	float4 tangent 	: TANGENT;
};

struct FragmentInputs
{
	float4 position 	: SV_POSITION;
	float4 color 		: COLOR;
	float2 uvs 			: UV;
	float3 position_WS 	: POSITION_WS;
	float3 normal_WS 	: NORMAL_WS;
	float3 tangent_WS 	: TANGENT_WS;
	float3 bitangent_WS : BITANGENT_WS;
	float3 position_VS 	: POSITION_VS;
};



// Entry Point --------------------------------------------------
FragmentInputs Main_VertexShader(VertexInputs input)
{
	float4 position_LS = float4(input.position, 1.0f);
	float4 position_WS = mul(MODEL, position_LS);
	float4 position_VS = mul(VIEW, position_WS);
	float4 position_CS = mul(PROJECTION, position_VS);
	
	
	// Tangent frame
	float4 normal_LS = float4(input.normal, 0.0f);
	float4 normal_WS = normalize(mul(MODEL, normal_LS));
	
	float4 tangent_LS = float4(input.tangent.xyz, 0.0f);
	float4 tangent_WS = normalize(mul(MODEL, tangent_LS));
	
	// Cross for the bitangent vector and use the packed sign provided in the input.tangent.w(1 or -1)
	float3 bitangent_WS = normalize(input.tangent.w * cross(tangent_WS.xyz, normal_WS.xyz));
	
	
	FragmentInputs outputs;
	outputs.position 	= position_CS;
	outputs.color 		= input.color;
	outputs.uvs 		= input.uvs;
	outputs.position_WS = position_WS.xyz;
	outputs.position_VS = position_VS.xyz;
	outputs.normal_WS 	= normal_WS.xyz;
	outputs.tangent_WS 	= tangent_WS.xyz;
	outputs.bitangent_WS = bitangent_WS;
    return outputs;
};