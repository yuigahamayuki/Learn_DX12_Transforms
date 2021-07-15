struct GSOutput
{
	float4 pos : SV_POSITION;
};

[maxvertexcount(3)]
void main(
	point float4 input[1] : SV_POSITION, 
	inout LineStream< GSOutput > output
)
{
	GSOutput element;
	element.pos = input[0];
	output.Append(element);
}