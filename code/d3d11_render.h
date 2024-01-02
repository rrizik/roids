#pragma once

static void
d3d_clear_color(RGBA color){
    begin_timed_function();
    d3d_context->ClearRenderTargetView(d3d_framebuffer_view, color.e);
    d3d_context->ClearDepthStencilView(d3d_depthbuffer_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

static void
d3d_draw_textured_quad(f32 x0, f32 y0, f32 x1, f32 y1, RGBA color, ID3D11ShaderResourceView** shader_resource){
    begin_timed_function();

    Vertex vertices[] = {
        { make_v3(x0, y0, 0.0f), color, make_v2(0.0f, 0.0f)},
        { make_v3(x1, y0, 0.0f), color, make_v2(1.0f, 0.0f)},
        { make_v3(x0, y1, 0.0f), color, make_v2(0.0f, 1.0f)},
        { make_v3(x1, y1, 0.0f), color, make_v2(1.0f, 1.0f)},
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

static void
d3d_draw_text(f32 x0, f32 y0, f32 x1, f32 y1, f32 s0, f32 t0, f32 s1, f32 t1, RGBA color, ID3D11ShaderResourceView** shader_resource){
    Vertex vertices[] = {
        { make_v3(x0, y0, 0.0f), color, make_v2(s0, t0)},
        { make_v3(x1, y0, 0.0f), color, make_v2(s1, t0)},
        { make_v3(x0, y1, 0.0f), color, make_v2(s0, t1)},
        { make_v3(x1, y1, 0.0f), color, make_v2(s1, t1)},
        //{ make_v3(x0, y0, 0.0f), color, make_v2(s0, t1)},
        //{ make_v3(x1, y0, 0.0f), color, make_v2(s1, t1)},
        //{ make_v3(x0, y1, 0.0f), color, make_v2(s0, t0)},
        //{ make_v3(x1, y1, 0.0f), color, make_v2(s1, t0)},
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
    d3d_context->OMSetDepthStencilState(d3d_depthstencil_state, 0);
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

    d3d_context->DrawIndexed(array_count(indices), 0, 0);
}

static void
d3d_draw_textured_cube_instanced(ID3D11ShaderResourceView** shader_resource){
    begin_timed_function();

    static Vertex vertices[] = {
        { make_v3(-20.0f, -20.0f,  20.0f), WHITE, make_v2(0.0f, 0.0f) },
        { make_v3( 20.0f, -20.0f,  20.0f), WHITE, make_v2(0.0f, 1.0f) },
        { make_v3(-20.0f,  20.0f,  20.0f), WHITE, make_v2(1.0f, 0.0f) },
        { make_v3( 20.0f,  20.0f,  20.0f), WHITE, make_v2(1.0f, 1.0f) },

        { make_v3(-20.0f, -20.0f, -20.0f), WHITE, make_v2(0.0f, 0.0f) },
        { make_v3(-20.0f,  20.0f, -20.0f), WHITE, make_v2(0.0f, 1.0f) },
        { make_v3( 20.0f, -20.0f, -20.0f), WHITE, make_v2(1.0f, 0.0f) },
        { make_v3( 20.0f,  20.0f, -20.0f), WHITE, make_v2(1.0f, 1.0f) },

        { make_v3(-20.0f,  20.0f, -20.0f), WHITE, make_v2(0.0f, 0.0f) },
        { make_v3(-20.0f,  20.0f,  20.0f), WHITE, make_v2(0.0f, 1.0f) },
        { make_v3( 20.0f,  20.0f, -20.0f), WHITE, make_v2(1.0f, 0.0f) },
        { make_v3( 20.0f,  20.0f,  20.0f), WHITE, make_v2(1.0f, 1.0f) },

        { make_v3(-20.0f, -20.0f, -20.0f), WHITE, make_v2(0.0f, 0.0f) },
        { make_v3( 20.0f, -20.0f, -20.0f), WHITE, make_v2(0.0f, 1.0f) },
        { make_v3(-20.0f, -20.0f,  20.0f), WHITE, make_v2(1.0f, 0.0f) },
        { make_v3( 20.0f, -20.0f,  20.0f), WHITE, make_v2(1.0f, 1.0f) },

        { make_v3( 20.0f, -20.0f, -20.0f), WHITE, make_v2(0.0f, 0.0f) },
        { make_v3( 20.0f,  20.0f, -20.0f), WHITE, make_v2(0.0f, 1.0f) },
        { make_v3( 20.0f, -20.0f,  20.0f), WHITE, make_v2(1.0f, 0.0f) },
        { make_v3( 20.0f,  20.0f,  20.0f), WHITE, make_v2(1.0f, 1.0f) },

        { make_v3(-20.0f, -20.0f, -20.0f), WHITE, make_v2(0.0f, 0.0f) },
        { make_v3(-20.0f, -20.0f,  20.0f), WHITE, make_v2(0.0f, 1.0f) },
        { make_v3(-20.0f,  20.0f, -20.0f), WHITE, make_v2(1.0f, 0.0f) },
        { make_v3(-20.0f,  20.0f,  20.0f), WHITE, make_v2(1.0f, 1.0f) },
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

    d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, d3d_depthbuffer_view);
    d3d_context->OMSetDepthStencilState(d3d_depthstencil_state, 0);
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
