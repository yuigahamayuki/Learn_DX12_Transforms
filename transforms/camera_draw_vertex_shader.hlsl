struct GSInput
{
	float3 center_pos : POSITION;
	float3 look_direction : TEXCOORD0;
	float3 right_direction : TEXCOOR1;
	float3 up_direction : TEXCOOR2;
};

GSInput main(GSInput input)
{
	return input;
}