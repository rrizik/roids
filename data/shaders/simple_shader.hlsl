cbuffer ConstantBuffer{
    row_major float4x4 view;
    row_major float4x4 projection;
}

Texture2D my_texture;
SamplerState my_sampler;

struct InputLayout{
    float3 vertex_position: POSITION;
    float2 tex: TEXCOORD;
    row_major float4x4 model_transform: TRANSFORM;
};

struct VS_Output{
    float4 pos: SV_POSITION;
    float2 tex: TEXCOORD;
};

VS_Output vs_main(InputLayout input){
    VS_Output output;


    // todo(rr): verify variable names are correct. What is result?
    float4 world_space = mul(float4(input.vertex_position, 1), input.model_transform);
    float4 camera_space = mul(world_space, view);
    float4 result = mul(camera_space, projection);
    output.pos = result;

    output.tex = input.tex;
    return output;
}

float4 ps_main(VS_Output output) : SV_TARGET{
    return my_texture.Sample(my_sampler, output.tex);
}
