#ifndef D3D11_RENDER_C
#define D3D11_RENDER_C

// todo: move these transformations to base files
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
push_clear_color(Arena* arena, RGBA color){
    RenderCommand* command = push_struct(arena, RenderCommand);
    command->type = RenderCommandType_ClearColor;
    command->color = color;
}

static void
push_quad(Arena* arena, v2 p0, v2 p1, v2 p2, v2 p3, RGBA color){
    RenderCommand* command = push_struct(arena, RenderCommand);
    command->type = RenderCommandType_Quad;
    command->color = color;
    command->p0 = p0;
    command->p1 = p1;
    command->p2 = p2;
    command->p3 = p3;
}

static void
push_text(Arena* arena, Font font, String8 text, f32 x, f32 y, RGBA color){
    RenderCommand* command = push_struct(arena, RenderCommand);
    command->type = RenderCommandType_Text;
    command->x = x;
    command->y = y;
    command->color = color;
    command->font = font;
    command->text = text;
}

static void
push_texture(Arena* arena, v2 p0, v2 p1, v2 p2, v2 p3, RGBA color, ID3D11ShaderResourceView** texture){
    RenderCommand* command = push_struct(arena, RenderCommand);
    command->type = RenderCommandType_Texture;
    command->color = color;
    command->p0 = p0;
    command->p1 = p1;
    command->p2 = p2;
    command->p3 = p3;
    command->texture = texture;
}

static void
draw_commands(Arena* commands){
    void* at = commands->base;
    void* end = (u8*)commands->base + commands->at;

    while(at != end){
        RenderCommand* command = (RenderCommand*)at;

        switch(command->type){
            case RenderCommandType_ClearColor:{
                d3d_clear_color(command->color);
            } break;
            case RenderCommandType_Quad:{
                d3d_draw_quad(command->p0, command->p1, command->p2, command->p3, command->color);
            } break;
            case RenderCommandType_Text:{
                d3d_draw_text(command->font, command->x, command->y, command->color, command->text);
            } break;
            case RenderCommandType_Texture:{
                d3d_draw_texture(command->p0, command->p1, command->p2, command->p3, command->color, command->texture);
            } break;
        }
		at = (u8*)at + sizeof(RenderCommand);
    }
}

static void
d3d_clear_color(RGBA color){
    d3d_context->ClearRenderTargetView(d3d_framebuffer_view, color.e);
    //d3d_context->ClearDepthStencilView(d3d_depthbuffer_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

static void
d3d_draw_quad(v2 p0, v2 p1, v2 p2, v2 p3, RGBA color){

    RGBA linear_color = srgb_to_linear(color); // gamma correction
    Vertex2 vertices[] = {
        { p0, linear_color },
        { p1, linear_color },
        { p2, linear_color },

        { p0, linear_color },
        { p2, linear_color },
        { p3, linear_color },
    };

    //----vertex buffer----
    {
        D3D11_MAPPED_SUBRESOURCE resource;
        hr = d3d_context->Map(d3d_vertex_buffer_8mb, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        assert_hr(hr);

        memcpy(resource.pData, vertices, sizeof(Vertex2) * array_count(vertices));
        d3d_context->Unmap(d3d_vertex_buffer_8mb, 0);

        ID3D11Buffer* buffers[] = {d3d_vertex_buffer_8mb};
        u32 strides[] = {sizeof(Vertex2)};
        u32 offset[] = {0};

        d3d_context->IASetVertexBuffers(0, 1, buffers, strides, offset);
    }

    //-------------------------------------------------------------------

    d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d_context->PSSetSamplers(0, 1, &d3d_sampler_state);

    d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, 0);
    d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xFFFFFFFF);

    //d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, d3d_depthbuffer_view);
    //D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {};
    //depth_stencil_desc.DepthEnable = false;
    //depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    //depth_stencil_desc.DepthFunc      = D3D11_COMPARISON_LESS;
    //hr = d3d_device->CreateDepthStencilState(&depth_stencil_desc, &d3d_depthstencil_state);
    //d3d_context->OMSetDepthStencilState(d3d_depthstencil_state, 0);

    D3D11_RASTERIZER_DESC1 rasterizer_desc = {};
    rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode = D3D11_CULL_BACK;
    hr = d3d_device->CreateRasterizerState1(&rasterizer_desc, &d3d_rasterizer_state);
    d3d_context->RSSetState(d3d_rasterizer_state);

    d3d_context->VSSetConstantBuffers(0, 1, &d3d_constant_buffer);

    d3d_context->RSSetViewports(1, &d3d_viewport);
    d3d_context->IASetInputLayout(d3d_2d_quad_il);
    d3d_context->VSSetShader(d3d_2d_quad_vs, 0, 0);
    d3d_context->PSSetShader(d3d_2d_quad_ps, 0, 0);

    d3d_context->Draw(6, 0);
}

// todo: pass in optional UV (x, y) that is added to each UV xy. Look at JB image as example
static void
d3d_draw_texture(v2 p0, v2 p1, v2 p2, v2 p3, RGBA color, ID3D11ShaderResourceView** texture){

    RGBA linear_color = srgb_to_linear(color); // gamma correction
    Vertex3 vertices[] = {
        { p0, linear_color, make_v2(0.0f, 0.0f)},
        { p1, linear_color, make_v2(1.0f, 0.0f)},
        { p2, linear_color, make_v2(1.0f, 1.0f)},

        { p0, linear_color, make_v2(0.0f, 0.0f)},
        { p2, linear_color, make_v2(1.0f, 1.0f)},
        { p3, linear_color, make_v2(0.0f, 1.0f)},
    };

    //----vertex buffer----
    {
        D3D11_MAPPED_SUBRESOURCE resource;
        hr = d3d_context->Map(d3d_vertex_buffer_8mb, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        assert_hr(hr);

        memcpy(resource.pData, vertices, sizeof(Vertex3) * array_count(vertices));
        d3d_context->Unmap(d3d_vertex_buffer_8mb, 0);

        ID3D11Buffer* buffers[] = {d3d_vertex_buffer_8mb};
        u32 strides[] = {sizeof(Vertex3)};
        u32 offset[] = {0};

        d3d_context->IASetVertexBuffers(0, 1, buffers, strides, offset);
    }

    //-------------------------------------------------------------------

    d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d_context->PSSetSamplers(0, 1, &d3d_sampler_state);
    d3d_context->PSSetShaderResources(0, 1, texture);

    d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, 0);
    d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xFFFFFFFF);

    //d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, d3d_depthbuffer_view);
    //D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {};
    //depth_stencil_desc.DepthEnable = false;
    //depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    //depth_stencil_desc.DepthFunc      = D3D11_COMPARISON_LESS;
    //hr = d3d_device->CreateDepthStencilState(&depth_stencil_desc, &d3d_depthstencil_state);
    //d3d_context->OMSetDepthStencilState(d3d_depthstencil_state, 0);

    D3D11_RASTERIZER_DESC1 rasterizer_desc = {};
    rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode = D3D11_CULL_BACK;
    hr = d3d_device->CreateRasterizerState1(&rasterizer_desc, &d3d_rasterizer_state);
    d3d_context->RSSetState(d3d_rasterizer_state);

    d3d_context->VSSetConstantBuffers(0, 1, &d3d_constant_buffer);

    d3d_context->RSSetViewports(1, &d3d_viewport);
    d3d_context->IASetInputLayout(d3d_2d_textured_il);
    d3d_context->VSSetShader(d3d_2d_textured_vs, 0, 0);
    d3d_context->PSSetShader(d3d_2d_textured_ps, 0, 0);

    d3d_context->Draw(6, 0);
}

static void d3d_draw_text(Font font, f32 x, f32 y, RGBA color, String8 text){
    RGBA linear_color = srgb_to_linear(color);

    ScratchArena scratch = begin_scratch(0);
    u64 allocation_size = text.size * 6;
    Vertex3* buffer = push_array(scratch.arena, Vertex3, allocation_size);
    Vertex3* vertex = buffer;

    f32 start_x = x;
    f32 y_offset = 0;
    stbtt_aligned_quad quad;
    for(s32 i=0; i < text.size; ++i){
        u8* character = text.str + i;
        if(*character == '\n'){
            y_offset += (f32)font.vertical_offset * font.scale;
            x = start_x;
        }
        stbtt_GetPackedQuad(font.packed_chars, font.texture_w, font.texture_h, (*character) - font.first_char, &x, &y, &quad, 1);
        Rect rect = make_rect(quad.x0, quad.y0 + y_offset, quad.x1, quad.y1 + y_offset);
        Rect clip_rect = rect_clip_from_pixel_inverted(rect, make_v2s32(window.width, window.height));

        *vertex++ = { make_v2(clip_rect.x0, clip_rect.y1), linear_color, make_v2(quad.s0, quad.t1) };
        *vertex++ = { make_v2(clip_rect.x1, clip_rect.y1), linear_color, make_v2(quad.s1, quad.t1) };
        *vertex++ = { make_v2(clip_rect.x0, clip_rect.y0), linear_color, make_v2(quad.s0, quad.t0) };
        *vertex++ = { make_v2(clip_rect.x0, clip_rect.y0), linear_color, make_v2(quad.s0, quad.t0) };
        *vertex++ = { make_v2(clip_rect.x1, clip_rect.y1), linear_color, make_v2(quad.s1, quad.t1) };
        *vertex++ = { make_v2(clip_rect.x1, clip_rect.y0), linear_color, make_v2(quad.s1, quad.t0) };
    }

    //----vertex buffer----
    {
        D3D11_MAPPED_SUBRESOURCE resource;
        hr = d3d_context->Map(d3d_vertex_buffer_8mb, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        assert_hr(hr);

        memcpy(resource.pData, buffer, sizeof(Vertex3) * allocation_size);
        d3d_context->Unmap(d3d_vertex_buffer_8mb, 0);

        ID3D11Buffer* buffers[] = {d3d_vertex_buffer_8mb};
        u32 strides[] = {sizeof(Vertex3)};
        u32 offset[] = {0};

        d3d_context->IASetVertexBuffers(0, 1, buffers, strides, offset);
    }

    //-------------------------------------------------------------------

    d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d_context->PSSetSamplers(0, 1, &d3d_sampler_state);
    d3d_context->PSSetShaderResources(0, 1, &font.atlas.view);

    d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, 0);
    d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xFFFFFFFF);

    //d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, d3d_depthbuffer_view);
    //D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {};
    //depth_stencil_desc.DepthEnable = false;
    //depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    //depth_stencil_desc.DepthFunc      = D3D11_COMPARISON_LESS;
    //hr = d3d_device->CreateDepthStencilState(&depth_stencil_desc, &d3d_depthstencil_state);
    //d3d_context->OMSetDepthStencilState(d3d_depthstencil_state, 0);

    D3D11_RASTERIZER_DESC1 rasterizer_desc = {};
    rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode = D3D11_CULL_FRONT;
    hr = d3d_device->CreateRasterizerState1(&rasterizer_desc, &d3d_rasterizer_state);
    d3d_context->RSSetState(d3d_rasterizer_state);

    d3d_context->RSSetViewports(1, &d3d_viewport);

    d3d_context->IASetInputLayout(d3d_2d_textured_il);
    d3d_context->VSSetShader(d3d_2d_textured_vs, 0, 0);
    d3d_context->PSSetShader(d3d_2d_textured_ps, 0, 0);

    d3d_context->Draw((UINT)allocation_size, 0);
    end_scratch(scratch);
}

static void
d3d_present(){
    d3d_swapchain->Present(1, 0);
}

#endif
