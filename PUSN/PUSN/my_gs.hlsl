struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float3 WorldPos : WORLD_POSITION;
};

[maxvertexcount(3)]
void main(triangle VS_OUTPUT input[3], inout TriangleStream< VS_OUTPUT > output)
{
	float3 a = input[1].WorldPos - input[0].WorldPos;
	float3 b = input[2].WorldPos - input[0].WorldPos;

	float3 normal = normalize(cross(a, b));

	for (uint i = 0; i < 3; i++)
	{
		input[i].Normal = normal;
		output.Append(input[i]);
	}
}