struct PSInput {
	float4 pos : SV_POSITION;
	float3 color : COLOR;
};

float4 main(PSInput ps_input) : SV_TARGET
{
	return float4(ps_input.color, 1.0f);
}