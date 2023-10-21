cbuffer ConstantBuffer{
    float4x4 transform;
}

Texture2D my_texture;
SamplerState my_sampler;

struct VS_Input{
    float3 pos: POSITION;
    float2 tex: TEXCOORD;
};

struct VS_Output{
    float4 pos: SV_POSITION;
    float2 tex: TEXCOORD;
};

VS_Output vs_main(VS_Input input){
    VS_Output output;

    output.pos = mul(float4(input.pos, 1.0f), transform);
    output.tex = input.tex;
    return output;
}

float4 ps_main(VS_Output output) : SV_TARGET{
    return my_texture.Sample(my_sampler, output.tex);
}


