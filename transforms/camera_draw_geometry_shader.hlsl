cbuffer SceneConstantBuffer : register(b0)
{
	float4x4 model;
	float4x4 view;
	float4x4 proj;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
};

[maxvertexcount(5)]
void main(
	point float4 input[1] : POSITION, 
	inout LineStream< GSOutput > output
)
{
	GSOutput element;
	element.pos = input[0];
	output.Append(element);
}