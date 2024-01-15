#ifndef D3D11_RENDER_CPP
#define D3D11_RENDER_CPP

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
d3d_clear_color(RGBA color){
    begin_timed_function();
    d3d_context->ClearRenderTargetView(d3d_framebuffer_view, color.e);
    d3d_context->ClearDepthStencilView(d3d_depthbuffer_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

static void
d3d_draw_quad(f32 x0, f32 y0, f32 x1, f32 y1, RGBA color){
    d3d_draw_quad_textured(x0, y0, x1, y1, color, &white_shader_resource);
}

// todo: pass in optional UV (x, y) that is added to each UV xy
static void
d3d_draw_quad_textured(f32 x0, f32 y0, f32 x1, f32 y1, RGBA color, ID3D11ShaderResourceView** shader_resource){
    begin_timed_function();

    RGBA linear_color = srgb_to_linear(color);
    Vertex vertices[] = {
        { make_v3(x0, y0, 0.0f), linear_color, make_v2(0.0f, 0.0f)},
        { make_v3(x1, y0, 0.0f), linear_color, make_v2(1.0f, 0.0f)},
        { make_v3(x0, y1, 0.0f), linear_color, make_v2(0.0f, 1.0f)},
        { make_v3(x1, y1, 0.0f), linear_color, make_v2(1.0f, 1.0f)},
    };

    s32 indices[] = {
        2, 3, 0, // top left
        0, 3, 1, // bottom right
    };

    //----vertex buffer----
    {
        D3D11_MAPPED_SUBRESOURCE resource;
        hr = d3d_context->Map(d3d_vertex_buffer_8mb, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        assert_hr(hr);

        memcpy(resource.pData, vertices, sizeof(Vertex) * array_count(vertices));
        d3d_context->Unmap(d3d_vertex_buffer_8mb, 0);

        ID3D11Buffer* buffers[] = {d3d_vertex_buffer_8mb};
        u32 strides[] = {sizeof(Vertex)};
        u32 offset[] = {0};

        d3d_context->IASetVertexBuffers(0, 1, buffers, strides, offset);
    }

    //----index buffer----
    {
        D3D11_MAPPED_SUBRESOURCE resource;
        hr = d3d_context->Map(d3d_index_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        assert_hr(hr);

        memcpy(resource.pData, indices, sizeof(s32) * array_count(indices));
        d3d_context->Unmap(d3d_index_buffer, 0);

        d3d_context->IASetIndexBuffer(d3d_index_buffer, DXGI_FORMAT_R32_UINT, 0);
    }

    //-------------------------------------------------------------------

    d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d_context->PSSetSamplers(0, 1, &d3d_sampler_state);
    d3d_context->PSSetShaderResources(0, 1, shader_resource);

    d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, d3d_depthbuffer_view);

    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {};
    depth_stencil_desc.DepthEnable = false;
    depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_stencil_desc.DepthFunc      = D3D11_COMPARISON_LESS;
    hr = d3d_device->CreateDepthStencilState(&depth_stencil_desc, &d3d_depthstencil_state);
    d3d_context->OMSetDepthStencilState(d3d_depthstencil_state, 0);

    d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xFFFFFFFF);

    D3D11_RASTERIZER_DESC1 rasterizer_desc = {};
    rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode = D3D11_CULL_BACK;
    hr = d3d_device->CreateRasterizerState1(&rasterizer_desc, &d3d_rasterizer_state);
    d3d_context->RSSetState(d3d_rasterizer_state);

    d3d_context->RSSetViewports(1, &d3d_viewport);
    d3d_context->IASetInputLayout(d3d_2d_textured_input_layout);
    d3d_context->VSSetShader(d3d_2d_textured_vertex_shader, 0, 0);
    d3d_context->PSSetShader(d3d_2d_textured_pixel_shader, 0, 0);

    d3d_context->DrawIndexed(array_count(indices), 0, 0);
}

static void d3d_draw_text(Font font, f32 x, f32 y, RGBA color, String8 text){
    RGBA linear_color = srgb_to_linear(color);

    ScratchArena scratch = begin_scratch(0);
    u64 allocation_size = text.size * 6;
    Vertex* buffer = push_array(scratch.arena, Vertex, allocation_size);
    Vertex* vertex = buffer;

    stbtt_aligned_quad quad;
    for(s32 i=0; i < text.size; ++i){
        u8* character = text.str + i;
        stbtt_GetPackedQuad(font.packed_chars, font.texture_w, font.texture_h, (*character) - font.first_char, &x, &y, &quad, 1);
        Rect rect = make_rect(quad.x0, quad.y0, quad.x1, quad.y1);
        Rect clip_rect = rect_pixel_to_clip_inverted(rect, resolution);

        *vertex++ = { make_v3(clip_rect.x0, clip_rect.y1, 0.0f), linear_color, make_v2(quad.s0, quad.t1) };
        *vertex++ = { make_v3(clip_rect.x1, clip_rect.y1, 0.0f), linear_color, make_v2(quad.s1, quad.t1) };
        *vertex++ = { make_v3(clip_rect.x0, clip_rect.y0, 0.0f), linear_color, make_v2(quad.s0, quad.t0) };
        *vertex++ = { make_v3(clip_rect.x0, clip_rect.y0, 0.0f), linear_color, make_v2(quad.s0, quad.t0) };
        *vertex++ = { make_v3(clip_rect.x1, clip_rect.y1, 0.0f), linear_color, make_v2(quad.s1, quad.t1) };
        *vertex++ = { make_v3(clip_rect.x1, clip_rect.y0, 0.0f), linear_color, make_v2(quad.s1, quad.t0) };
    }

    //----vertex buffer----
    {
        D3D11_MAPPED_SUBRESOURCE resource;
        hr = d3d_context->Map(d3d_vertex_buffer_8mb, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        assert_hr(hr);

        memcpy(resource.pData, buffer, sizeof(Vertex) * allocation_size);
        d3d_context->Unmap(d3d_vertex_buffer_8mb, 0);

        ID3D11Buffer* buffers[] = {d3d_vertex_buffer_8mb};
        u32 strides[] = {sizeof(Vertex)};
        u32 offset[] = {0};

        d3d_context->IASetVertexBuffers(0, 1, buffers, strides, offset);
    }

    //-------------------------------------------------------------------

    d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d_context->PSSetSamplers(0, 1, &d3d_sampler_state);
    d3d_context->PSSetShaderResources(0, 1, &font.atlas.view);


    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {};
    depth_stencil_desc.DepthEnable = false;
    depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_stencil_desc.DepthFunc      = D3D11_COMPARISON_LESS;
    hr = d3d_device->CreateDepthStencilState(&depth_stencil_desc, &d3d_depthstencil_state);
    d3d_context->OMSetDepthStencilState(d3d_depthstencil_state, 0);

    d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, d3d_depthbuffer_view);
    d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xFFFFFFFF);

    D3D11_RASTERIZER_DESC1 rasterizer_desc = {};
    rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode = D3D11_CULL_FRONT;
    hr = d3d_device->CreateRasterizerState1(&rasterizer_desc, &d3d_rasterizer_state);
    d3d_context->RSSetState(d3d_rasterizer_state);

    d3d_context->RSSetViewports(1, &d3d_viewport);

    d3d_context->IASetInputLayout(d3d_2d_textured_input_layout);
    d3d_context->VSSetShader(d3d_2d_textured_vertex_shader, 0, 0);
    d3d_context->PSSetShader(d3d_2d_textured_pixel_shader, 0, 0);

    d3d_context->Draw((UINT)allocation_size, 0);
    end_scratch(scratch);
}

static void
d3d_draw_textured_cube_instanced(ID3D11ShaderResourceView** shader_resource){
    begin_timed_function();

    RGBA linear_color = srgb_to_linear(WHITE);
    static Vertex vertices[] = {
        { make_v3(-20.0f, -20.0f,  20.0f), linear_color, make_v2(0.0f, 0.0f) },
        { make_v3( 20.0f, -20.0f,  20.0f), linear_color, make_v2(0.0f, 1.0f) },
        { make_v3(-20.0f,  20.0f,  20.0f), linear_color, make_v2(1.0f, 0.0f) },
        { make_v3( 20.0f,  20.0f,  20.0f), linear_color, make_v2(1.0f, 1.0f) },

        { make_v3(-20.0f, -20.0f, -20.0f), linear_color, make_v2(0.0f, 0.0f) },
        { make_v3(-20.0f,  20.0f, -20.0f), linear_color, make_v2(0.0f, 1.0f) },
        { make_v3( 20.0f, -20.0f, -20.0f), linear_color, make_v2(1.0f, 0.0f) },
        { make_v3( 20.0f,  20.0f, -20.0f), linear_color, make_v2(1.0f, 1.0f) },

        { make_v3(-20.0f,  20.0f, -20.0f), linear_color, make_v2(0.0f, 0.0f) },
        { make_v3(-20.0f,  20.0f,  20.0f), linear_color, make_v2(0.0f, 1.0f) },
        { make_v3( 20.0f,  20.0f, -20.0f), linear_color, make_v2(1.0f, 0.0f) },
        { make_v3( 20.0f,  20.0f,  20.0f), linear_color, make_v2(1.0f, 1.0f) },

        { make_v3(-20.0f, -20.0f, -20.0f), linear_color, make_v2(0.0f, 0.0f) },
        { make_v3( 20.0f, -20.0f, -20.0f), linear_color, make_v2(0.0f, 1.0f) },
        { make_v3(-20.0f, -20.0f,  20.0f), linear_color, make_v2(1.0f, 0.0f) },
        { make_v3( 20.0f, -20.0f,  20.0f), linear_color, make_v2(1.0f, 1.0f) },

        { make_v3( 20.0f, -20.0f, -20.0f), linear_color, make_v2(0.0f, 0.0f) },
        { make_v3( 20.0f,  20.0f, -20.0f), linear_color, make_v2(0.0f, 1.0f) },
        { make_v3( 20.0f, -20.0f,  20.0f), linear_color, make_v2(1.0f, 0.0f) },
        { make_v3( 20.0f,  20.0f,  20.0f), linear_color, make_v2(1.0f, 1.0f) },

        { make_v3(-20.0f, -20.0f, -20.0f), linear_color, make_v2(0.0f, 0.0f) },
        { make_v3(-20.0f, -20.0f,  20.0f), linear_color, make_v2(0.0f, 1.0f) },
        { make_v3(-20.0f,  20.0f, -20.0f), linear_color, make_v2(1.0f, 0.0f) },
        { make_v3(-20.0f,  20.0f,  20.0f), linear_color, make_v2(1.0f, 1.0f) },
    };

    static u32 indices[] = {
        0, 1, 2,    // side 1
        2, 1, 3,
        4, 5, 6,    // side 2
        6, 5, 7,
        8, 9, 10,    // side 3
        10, 9, 11,
        12, 13, 14,    // side 4
        14, 13, 15,
        16, 17, 18,    // side 5
        18, 17, 19,
        20, 21, 22,    // side 6
        22, 21, 23,
    };

    //----instance buffer----
    {
        D3D11_MAPPED_SUBRESOURCE resource;
        hr = d3d_context->Map(d3d_instance_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        assert_hr(hr);

        memcpy(resource.pData, cube_instances, sizeof(InstanceData) * array_count(cube_instances));
        d3d_context->Unmap(d3d_instance_buffer, 0);

    }

    //----vertex buffer----
    {
        D3D11_MAPPED_SUBRESOURCE resource;
        hr = d3d_context->Map(d3d_vertex_buffer_8mb, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        assert_hr(hr);

        memcpy(resource.pData, vertices, sizeof(Vertex) * array_count(vertices));
        d3d_context->Unmap(d3d_vertex_buffer_8mb, 0);

        ID3D11Buffer* buffers[] = {d3d_vertex_buffer_8mb, d3d_instance_buffer};
        u32 strides[] = {sizeof(Vertex), sizeof(InstanceData)};
        u32 offset[] = {0, 0};

        d3d_context->IASetVertexBuffers(0, 2, buffers, strides, offset);
    }

    //----index buffer----
    {
        D3D11_MAPPED_SUBRESOURCE resource;
        hr = d3d_context->Map(d3d_index_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        assert_hr(hr);

        memcpy(resource.pData, indices, sizeof(s32) * array_count(indices));
        d3d_context->Unmap(d3d_index_buffer, 0);

        d3d_context->IASetIndexBuffer(d3d_index_buffer, DXGI_FORMAT_R32_UINT, 0);
    }

    //----state----
    d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d_context->PSSetSamplers(0, 1, &d3d_sampler_state);
    d3d_context->PSSetShaderResources(0, 1, shader_resource);

    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {};
    depth_stencil_desc.DepthEnable = true;
    depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_stencil_desc.DepthFunc      = D3D11_COMPARISON_LESS;
    hr = d3d_device->CreateDepthStencilState(&depth_stencil_desc, &d3d_depthstencil_state);
    d3d_context->OMSetDepthStencilState(d3d_depthstencil_state, 0);

    d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, d3d_depthbuffer_view);
    d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xFFFFFFFF);

    D3D11_RASTERIZER_DESC1 rasterizer_desc = {};
    rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode = D3D11_CULL_BACK;

    hr = d3d_device->CreateRasterizerState1(&rasterizer_desc, &d3d_rasterizer_state);
    d3d_context->RSSetState(d3d_rasterizer_state);
    d3d_context->RSSetViewports(1, &d3d_viewport);

    d3d_context->IASetInputLayout(d3d_3d_input_layout);
    d3d_context->VSSetShader(d3d_3d_vertex_shader, 0, 0);
    d3d_context->PSSetShader(d3d_3d_pixel_shader, 0, 0);

    d3d_context->DrawIndexedInstanced(array_count(indices), 3, 0, 0, 0);
}

static void
d3d_present(){
    d3d_swapchain->Present(1, 0);
}

#endif
