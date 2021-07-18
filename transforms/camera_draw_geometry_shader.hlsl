cbuffer SceneConstantBuffer : register(b0)
{
	float4x4 model;
	float4x4 view;
	float4x4 proj;
};

struct GSInput
{
	float3 center_pos : POSITION;
	float3 look_direction : TEXCOORD0;
	float3 right_direction : TEXCOOR1;
	float3 up_direction : TEXCOOR2;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
};

[maxvertexcount(10)]
void main(
	point GSInput input[1] : POSITION,
	inout LineStream< GSOutput > output_stream
)
{
	const float horizontal_length = 0.16f;
	const float vertical_length = 0.09f;
	const float look_length = 0.2f;

	float3 upper_right = input[0].center_pos + horizontal_length * input[0].right_direction + vertical_length * input[0].up_direction;
	float3 bottom_right = input[0].center_pos + horizontal_length * input[0].right_direction - vertical_length * input[0].up_direction;
	float3 upper_left = input[0].center_pos - horizontal_length * input[0].right_direction + vertical_length * input[0].up_direction;
	float3 bottom_left = input[0].center_pos - horizontal_length * input[0].right_direction - vertical_length * input[0].up_direction;
	float3 pyramid_top_point = input[0].center_pos + look_length * input[0].look_direction;

	GSOutput gs_output;
	gs_output.pos = float4(bottom_left, 1.0f);
	gs_output.pos = mul(gs_output.pos, view);
	gs_output.pos = mul(gs_output.pos, proj);
	output_stream.Append(gs_output);

	gs_output.pos = float4(upper_left, 1.0f);
	gs_output.pos = mul(gs_output.pos, view);
	gs_output.pos = mul(gs_output.pos, proj);
	output_stream.Append(gs_output);

	gs_output.pos = float4(upper_right, 1.0f);
	gs_output.pos = mul(gs_output.pos, view);
	gs_output.pos = mul(gs_output.pos, proj);
	output_stream.Append(gs_output);

	gs_output.pos = float4(bottom_right, 1.0f);
	gs_output.pos = mul(gs_output.pos, view);
	gs_output.pos = mul(gs_output.pos, proj);
	output_stream.Append(gs_output);

	gs_output.pos = float4(bottom_left, 1.0f);
	gs_output.pos = mul(gs_output.pos, view);
	gs_output.pos = mul(gs_output.pos, proj);
	output_stream.Append(gs_output);

	gs_output.pos = float4(pyramid_top_point, 1.0f);
	gs_output.pos = mul(gs_output.pos, view);
	gs_output.pos = mul(gs_output.pos, proj);
	output_stream.Append(gs_output);
	
	gs_output.pos = float4(upper_left, 1.0f);
	gs_output.pos = mul(gs_output.pos, view);
	gs_output.pos = mul(gs_output.pos, proj);
	output_stream.Append(gs_output);

	output_stream.RestartStrip();

	gs_output.pos = float4(upper_right, 1.0f);
	gs_output.pos = mul(gs_output.pos, view);
	gs_output.pos = mul(gs_output.pos, proj);
	output_stream.Append(gs_output);

	gs_output.pos = float4(pyramid_top_point, 1.0f);
	gs_output.pos = mul(gs_output.pos, view);
	gs_output.pos = mul(gs_output.pos, proj);
	output_stream.Append(gs_output);

	gs_output.pos = float4(bottom_right, 1.0f);
	gs_output.pos = mul(gs_output.pos, view);
	gs_output.pos = mul(gs_output.pos, proj);
	output_stream.Append(gs_output);
}