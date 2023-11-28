struct VertexInput{
    float3 pos: POS;
    float4 col: COL;
};

struct VertexOutput{
    float4 pos: SV_POSITION;
    float4 col: COLOR;
};

VertexOutput vs_main(VertexInput vertex){
    VertexOutput result;

    result.pos = float4(vertex.pos, 1.0);
    result.col = vertex.col;
    return result;
}

float4 ps_main(VertexOutput vertex) : SV_TARGET{
    return vertex.col;
}
