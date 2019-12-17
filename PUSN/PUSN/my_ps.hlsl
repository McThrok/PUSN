cbuffer buffer: register(b0)
{
	float3 color;
};

struct PS_INPUT
{
	float4 inPosition : SV_POSITION;
	float3 inNormal : NORMAL;
	float3 inWorldPos : WORLD_POSITION;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float3 ambient = float3(0.05,0.05,0.05);// float3(0.2 ,0.2, 0.2);
	float3 diffuse = float3(0.6 ,0.6, 0.6);
	float3 lightPosition = float3(50, 0, 150);

	float3 vectorToLight = normalize(lightPosition - input.inWorldPos);
	float3 diff = max(dot(vectorToLight, input.inNormal), 0);
	float3 finalColor = color * (ambient + diffuse* diff);
	return float4(finalColor, 1.0f);
}