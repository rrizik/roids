#ifndef DRAW_C
#define DRAW_C

static RGBA
srgb_to_linear_approx(RGBA value){
    RGBA result = {
        .r = square_f32(value.r),
        .g = square_f32(value.g),
        .b = square_f32(value.b),
        .a = value.a,
    };
    return(result);
}

static RGBA
linear_to_srgb_approx(RGBA value){
    RGBA result = {
        .r = sqrt_f32(value.r),
        .g = sqrt_f32(value.g),
        .b = sqrt_f32(value.b),
        .a = value.a,
    };
    return(result);
}

static RGBA
srgb_to_linear(RGBA value){
    RGBA result = {0};
    result.a = value.a;

    if(value.r < 0.04045f){
        result.r = value.r / 12.92f;
    }
    else{
        result.r = powf(((value.r + 0.055f) / (1.055f)), 2.4f);
    }

    if(value.g < 0.04045f){
        result.g = value.g / 12.92f;
    }
    else{
        result.g = powf(((value.g + 0.055f) / (1.055f)), 2.4f);
    }

    if(value.b < 0.04045f){
        result.b = value.b / 12.92f;
    }
    else{
        result.b = powf(((value.b + 0.055f) / (1.055f)), 2.4f);
    }
    return(result);
}

static void
init_render_commands(Arena* arena_b, Assets* assets){
    rb_arena = arena_b;
    r_assets = assets;
}

static void
set_texture(Texture* texture){
    r_texture = texture;
}

static Texture*
get_texture(void){
    return(r_texture);
}

// note: quads are always top-left, top-right, bottom-right, bottom-left order
static void
draw_quad(v2 p0, v2 p1, v2 p2, v2 p3, RGBA color){

    set_texture(&r_assets->textures[TextureAsset_White]);
    RenderBatch *batch = get_render_batch();

    RGBA linear_color = srgb_to_linear(color); // gamma correction
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p1, linear_color, make_v2(1.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_quad(v2 pos, v2 dim, RGBA color){

    set_texture(&r_assets->textures[TextureAsset_White]);
    RenderBatch *batch = get_render_batch();

    v2 p0 = pos;
    v2 p1 = make_v2(pos.x + dim.w, pos.y);
    v2 p2 = make_v2(pos.x + dim.w, pos.y + dim.h);
    v2 p3 = make_v2(pos.x, pos.y + dim.h);

    RGBA linear_color = srgb_to_linear(color); // gamma correction
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p1, linear_color, make_v2(1.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_quad(Rect rect, RGBA color){

    set_texture(&r_assets->textures[TextureAsset_White]);
    RenderBatch *batch = get_render_batch();

    v2 p0 = make_v2(rect.x0, rect.y0);
    v2 p1 = make_v2(rect.x1, rect.y0);
    v2 p2 = make_v2(rect.x1, rect.y1);
    v2 p3 = make_v2(rect.x0, rect.y1);

    RGBA linear_color = srgb_to_linear(color); // gamma correction
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p1, linear_color, make_v2(1.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_quad(Quad quad, RGBA color){

    set_texture(&r_assets->textures[TextureAsset_White]);
    RenderBatch *batch = get_render_batch();

    RGBA linear_color = srgb_to_linear(color); // gamma correction
    batch->buffer[batch->count++] = { quad.p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { quad.p1, linear_color, make_v2(1.0f, 0.0f) };
    batch->buffer[batch->count++] = { quad.p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { quad.p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { quad.p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { quad.p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_line(v2 p0, v2 p1, f32 width, RGBA color){

    set_texture(&r_assets->textures[TextureAsset_White]);
    RenderBatch *batch = get_render_batch();

    v2 dir = direction_v2(p0, p1);
    v2 perp = perpendicular(dir);
    v2 p2 = p1 + (perp * width);
    v2 p3 = p0 + (perp * width);

    RGBA linear_color = srgb_to_linear(color); // gamma correction
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p1, linear_color, make_v2(1.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_texture(u32 texture, v2 p0, v2 p1, v2 p2, v2 p3, RGBA color){

    RenderBatch *batch = get_render_batch();

    RGBA linear_color = srgb_to_linear(color); // gamma correction
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p1, linear_color, make_v2(1.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_text(Font* font, String8 text, v2 pos, RGBA color){

    set_texture(&font->texture);
    RenderBatch* batch = get_render_batch();
    RGBA linear_color = srgb_to_linear(color);

    f32 start_x = pos.x;
    f32 y_offset = 0;
    stbtt_aligned_quad quad;
    u32 vertex_count = 0;
    for(s32 i=0; i < text.size; ++i){
        u8* character = text.str + i;
        if(*character == '\n'){
            y_offset += (f32)font->vertical_offset;
            pos.x = start_x;
        }
        else{
            stbtt_GetPackedQuad(font->packed_chars, font->texture_w, font->texture_h, (*character) - font->first_char, &pos.x, &pos.y, &quad, 1);
            v2 p0 = make_v2(quad.x0, quad.y0 + y_offset);
            v2 p1 = make_v2(quad.x1, quad.y0 + y_offset);
            v2 p2 = make_v2(quad.x1, quad.y1 + y_offset);
            v2 p3 = make_v2(quad.x0, quad.y1 + y_offset);

            // todo: remove this
            //g_angle += 1 * (f32)clock.dt;
            //v2 origin = make_v2((p0.x + p2.x)/2, (p0.y + p2.y)/2);
            //p0 = rotate_point_deg(p0, g_angle, origin);
            //p1 = rotate_point_deg(p1, g_angle, origin);
            //p2 = rotate_point_deg(p2, g_angle, origin);
            //p3 = rotate_point_deg(p3, g_angle, origin);

            batch->buffer[batch->count++] = { p0, linear_color, make_v2(quad.s0, quad.t0) };
            batch->buffer[batch->count++] = { p1, linear_color, make_v2(quad.s1, quad.t0) };
            batch->buffer[batch->count++] = { p2, linear_color, make_v2(quad.s1, quad.t1) };
            batch->buffer[batch->count++] = { p0, linear_color, make_v2(quad.s0, quad.t0) };
            batch->buffer[batch->count++] = { p2, linear_color, make_v2(quad.s1, quad.t1) };
            batch->buffer[batch->count++] = { p3, linear_color, make_v2(quad.s0, quad.t1) };
        }
    }
}

static RenderBatch*
get_render_batch(void){
    Texture* texture = get_texture();

    RenderBatch *batch = render_batches.last;

    if(batch == 0 || batch->count >= batch->cap || batch->texture != texture){
        batch = push_array_zero(rb_arena, RenderBatch, 1);
        batch->buffer = push_array_zero(rb_arena, Vertex3, DEFAULT_BATCH_SIZE / sizeof(Vertex3));
        batch->cap = DEFAULT_BATCH_SIZE / sizeof(Vertex3);
        batch->count = 0;
        batch->texture = texture;
        batch->id = render_batches.count;
        if(render_batches.last == 0){
            render_batches.last = batch;
            render_batches.first = batch;
        }
        else{
            render_batches.last->next = batch;
            render_batches.last = batch;
        }
        render_batches.count++;
    }
    return(batch);
}


static void draw_render_batches(){
    for(RenderBatch* batch = render_batches.first; batch != 0; batch = batch->next){
        {
            D3D11_MAPPED_SUBRESOURCE resource;
            hr = d3d_context->Map(d3d_vertex_buffer_8mb, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
            assert_hr(hr);

            memcpy(resource.pData, batch->buffer, sizeof(Vertex3) * batch->count);
            d3d_context->Unmap(d3d_vertex_buffer_8mb, 0);

            ID3D11Buffer* buffers[] = {d3d_vertex_buffer_8mb};
            u32 strides[] = {sizeof(Vertex3)};
            u32 offset[] = {0};

            d3d_context->IASetVertexBuffers(0, 1, buffers, strides, offset);
        }

        //-------------------------------------------------------------------

        d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        d3d_context->PSSetSamplers(0, 1, &d3d_sampler_state);
        d3d_context->PSSetShaderResources(0, 1, &batch->texture->view);

        d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, 0);
        d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xFFFFFFFF);
        d3d_context->RSSetState(d3d_rasterizer_state);

        d3d_context->VSSetConstantBuffers(0, 1, &d3d_constant_buffer);

        d3d_context->RSSetViewports(1, &d3d_viewport);
        d3d_context->IASetInputLayout(d3d_2d_textured_il);
        d3d_context->VSSetShader(d3d_2d_textured_vs, 0, 0);
        d3d_context->PSSetShader(d3d_2d_textured_ps, 0, 0);

        d3d_context->Draw((u32)batch->count, 0);
    }
}

static void
render_batches_reset(void){
    render_batches.first = 0;
    render_batches.last = 0;
    render_batches.count = 0;
}
#endif
