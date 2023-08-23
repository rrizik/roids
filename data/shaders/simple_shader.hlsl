cbuffer ConstantBuffer{
    float4x4 transform;
}

float4 vs_main(float3 pos : POSITION) : SV_POSITION {
    return mul(float4(pos, 1.0f), transform);
}

cbuffer PSConstantBuffer{
    float4 face_colors[6];
};

float4 ps_main(uint tid : SV_PRIMITIVEID) : SV_TARGET {
    return face_colors[tid / 2];
}
