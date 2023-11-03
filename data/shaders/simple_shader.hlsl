cbuffer ConstantBuffer{
    float4x4 view;
    float4x4 projection;
}

Texture2D my_texture;
SamplerState my_sampler;

struct InputLayout{
    float3 pos: POSITION;
    float2 tex: TEXCOORD;
    float4x4 transform: TRANSFORM;
};

struct VS_Output{
    float4 pos: SV_POSITION;
    float2 tex: TEXCOORD;
};

VS_Output vs_main(InputLayout input){
    VS_Output output;

    float4 world_position = mul(float4(input.pos, 1), input.transform);
    float4 view_position = mul(view, world_position);
    output.pos = mul(projection, view_position);

    //float4 view_position = mul(view, input.pos);
    //output.pos = mul(projection, view_position);
    //float4x4 final_matrix = mul(view, projection);
    //output.pos = mul(float4(input.pos, 1), final_matrix);

    //output.pos = mul(mul(float4(input.pos, 1.0f), input.transform), transform);
    //output.pos = mul(float4(input.pos, 1.0f), mul(input.transform, transform));
    //output.pos = mul(float4(input.pos, 1.0f), input.transform);
    //output.pos = mul(float4(input.pos, 1.0f), transform);
    output.tex = input.tex;
    return output;
}

float4 ps_main(VS_Output output) : SV_TARGET{
    return my_texture.Sample(my_sampler, output.tex);
}
