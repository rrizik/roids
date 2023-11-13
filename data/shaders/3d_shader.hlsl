cbuffer ConstantBuffer{
    row_major float4x4 view;
    row_major float4x4 projection;
}

Texture2D my_texture;
SamplerState my_sampler;

struct VertexIn{
    float3 pos: POS;
    float4 col: COL;
    float2 tex: TEX;
    row_major float4x4 model_transform: TRANSFORM;
};

struct VertexOut{
    float4 pos: SV_POSITION;
    float2 tex: TEXCOORD;
};

VertexOut vs_main(VertexIn vertex){
    VertexOut output;

    // todo(rr): verify variable names are correct. What is result?
    float4 world_space = mul(float4(vertex.pos, 1), vertex.model_transform);
    float4 camera_space = mul(world_space, view);
    float4 result = mul(camera_space, projection);
    output.pos = result;

    output.tex = vertex.tex;
    return output;
}

float4 ps_main(VertexOut output) : SV_TARGET{
    return my_texture.Sample(my_sampler, output.tex);
}
