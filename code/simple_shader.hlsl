float4 vs_main(float2 pos : POSITION) : SV_POSITION
{
    return float4(pos.x, pos.y, 0.0f, 1.0f);
}

float4 ps_main() : SV_TARGET
{
    return float4(0.0f, 1.0f, 1.0f, 1.0f);
}
