Texture2D my_texture;
SamplerState my_sampler;

struct VertexInput{
    float3 pos: POS;
    float4 col: COL;
    float2 tex: TEX;
};

struct VertexOutput{
    float4 pos: SV_POSITION;
    float4 col: COLOR;
    float2 tex: TEXCOORD;
};

VertexOutput vs_main(VertexInput vertex){
    VertexOutput result;

    result.pos = float4(vertex.pos, 1);
    result.col = vertex.col;
    result.tex = vertex.tex;
    return result;
}

float4 ps_main(VertexOutput vertex) : SV_TARGET{
    float4 tex_color = my_texture.Sample(my_sampler, vertex.tex);
    //float4 result = tex_color * vertex.col;
    //float4 result = lerp(tex_color, vertex.col, 0.5);
    return tex_color;
}
