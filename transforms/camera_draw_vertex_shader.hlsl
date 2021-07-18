float4 main( float3 pos : POSITION ) : POSITION
{
	return float4(pos, 1.0f);
}