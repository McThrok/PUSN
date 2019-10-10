#pragma pack_matrix( row_major )

cbuffer perObjectBuffer : register(b0)
{
	float4x4 wvpMatrix;
	float4x4 worldMatrix;
};

struct VS_INPUT
{
	float3 inPos : POSITION;
	float2 inTexCoord : TEXCOORD;
	float3 inNormal : NORMAL;
};

struct VS_OUTPUT
{
	float4 outPosition : SV_POSITION;
	float3 outNormal : NORMAL;
	float3 outWorldPos : WORLD_POSITION;
};

Texture2D objTexture : TEXTURE: register(t0);
SamplerState objSamplerState : SAMPLER: register(s0);

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	float3 pos = input.inPos;
	 
	//if (input.inTexCoord != float2(-1, -1) 
	//	pos.y += objTexture.Sample(objSamplerState, input.inTexCoord);

	output.outPosition = mul(float4(pos, 1.0f), wvpMatrix);
	output.outNormal = normalize(mul(float4(input.inNormal, 0.0f), worldMatrix));
	output.outWorldPos = mul(float4(pos, 1.0f), worldMatrix);
	return output;
}