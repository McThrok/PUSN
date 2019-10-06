struct PS_INPUT
{
	float4 inPosition : SV_POSITION;
	float3 inNormal : NORMAL;
	float3 inWorldPos : WORLD_POSITION;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float3 ambient = float3(0.2 ,0.2, 0.2);
	float3 diffuse = float3(0.6 ,0.6, 0.6);
	float3 lightPosition = float3(0, 10, 0);
	float3 material_color = float3(1, 1, 1);

	float3 vectorToLight = normalize(lightPosition - input.inWorldPos);
	float3 diff = max(dot(vectorToLight, input.inNormal), 0);
	float3 finalColor = material_color * (ambient + diffuse* diff);
	return float4(finalColor, 1.0f);
}