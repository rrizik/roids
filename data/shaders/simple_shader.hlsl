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

    //float4x4 pv = mul(view, projection);
    //float4x4 pvm = mul(pv, input.model_transform);
    //float4 result = mul(pvm, float4(input.vertex_position, 1));
    //output.pos = result;

    // (((IN*M)*V)*P)
    //float4 world_position = mul(input.model_transform, float4(input.vertex_position, 1.0));
    //float4 view_position = mul(view, world_position);
    //float4 result = mul(projection, world_position);
    // (((P*V)*M)*IN)
    //float4x4 pv = mul(projection, view);
    //float4x4 pvm = mul(pv, input.model_transform);
    //float4 result = mul(pv, float4(input.vertex_position, 1));

    float4 world_space = mul(input.model_transform, float4(input.vertex_position, 1));
    float4 camera_space = mul(world_space, view);
    //float4 result = mul(camera_space, projection);
    float4 result = mul(float4(input.vertex_position, 1), mul(input.model_transform, mul(view, projection)));
    output.pos = result;

    // (((IN*M)*V)*P)
    //float4 inm = mul(float4(input.vertex_position, 1), input.model_transform);
    //float4 inmv = mul(inm, view);
    //float4 result = mul(inmv, projection);
    //output.pos = result;
    // camera multiplication
    //float4 world_position = mul(float4(input.vertex_position, 1), input.model_transform);
    //float4 view_position = mul(view, world_position);
    //output.pos = mul(projection, view_position);

    //output.pos = mul(mul(float4(input.vertex_position, 1.0f), input.transform), transform);
    //output.pos = mul(float4(input.vertex_position, 1.0f), mul(input.transform, transform));
    //output.pos = mul(float4(input.vertex_position, 1.0f), input.transform);
    //output.pos = mul(float4(input.vertex_position, 1.0f), transform);
    output.tex = input.tex;
    return output;
}

float4 ps_main(VS_Output output) : SV_TARGET{
    return my_texture.Sample(my_sampler, output.tex);
}
