#pragma once

static void
d3d_clear_color(RGBA color){
    begin_timed_function();
    d3d_context->ClearRenderTargetView(d3d_framebuffer_view, color.e);
    d3d_context->ClearDepthStencilView(d3d_depthbuffer_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

static void
d3d_draw_quad(f32 x0, f32 y0, f32 x1, f32 y1, RGBA color){
    begin_timed_function();

    Vertex vertices[] = {
        { make_v3(x0, y0,  0.0f), color },
        { make_v3(x1, y0,  0.0f), color },
        { make_v3(x0, y1,  0.0f), color },
        { make_v3(x1, y1,  0.0f), color },
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
    d3d_context->PSSetShaderResources(0, 1, &d3d_shader_resource);

    d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, d3d_depthbuffer_view);
    d3d_context->OMSetDepthStencilState(d3d_depthstencil_state, 0);
    d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xFFFFFFFF);

    d3d_context->RSSetState(d3d_rasterizer_state);
    d3d_context->RSSetViewports(1, &d3d_viewport);

    d3d_context->IASetInputLayout(d3d_2d_input_layout);
    d3d_context->VSSetShader(d3d_2d_vertex_shader, 0, 0);
    d3d_context->PSSetShader(d3d_2d_pixel_shader, 0, 0);

    d3d_context->DrawIndexed(array_count(indices), 0, 0);
}

static void d3d_draw_textured_quad(f32 x0, f32 y0, f32 x1, f32 y1, Bitmap* texture){
    begin_timed_function();

    Vertex vertices[] = {
        { make_v3(x0, y0, 0.0f), CLEAR, make_v2(0.0f, 0.0f)},
        { make_v3(x1, y0, 0.0f), CLEAR, make_v2(1.0f, 0.0f)},
        { make_v3(x0, y1, 0.0f), CLEAR, make_v2(0.0f, 1.0f)},
        { make_v3(x1, y1, 0.0f), CLEAR, make_v2(1.0f, 1.0f)},
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

    D3D11_TEXTURE2D_DESC texture_desc = {
        .Width = (u32)texture->width,
        .Height = (u32)texture->height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
        .SampleDesc = {1, 0},
        .Usage = D3D11_USAGE_IMMUTABLE,
        .BindFlags = D3D11_BIND_SHADER_RESOURCE,
    };

    D3D11_SUBRESOURCE_DATA data = {
        .pSysMem = texture->base,
        .SysMemPitch = (u32)texture->stride,
    };

    hr = d3d_device->CreateTexture2D(&texture_desc, &data, &d3d_texture);
    assert_hr(hr);

    hr = d3d_device->CreateShaderResourceView(d3d_texture, 0, &d3d_shader_resource);
    assert_hr(hr);

    d3d_context->PSSetShaderResources(0, 1, &d3d_shader_resource);
    //-------------------------------------------------------------------

    d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d_context->PSSetSamplers(0, 1, &d3d_sampler_state);
    d3d_context->PSSetShaderResources(0, 1, &d3d_shader_resource);

    d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, d3d_depthbuffer_view);
    d3d_context->OMSetDepthStencilState(d3d_depthstencil_state, 0);
    d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xFFFFFFFF);

    d3d_context->RSSetState(d3d_rasterizer_state);
    d3d_context->RSSetViewports(1, &d3d_viewport);

    d3d_context->IASetInputLayout(d3d_2d_textured_input_layout);
    d3d_context->VSSetShader(d3d_2d_textured_vertex_shader, 0, 0);
    d3d_context->PSSetShader(d3d_2d_textured_pixel_shader, 0, 0);

    d3d_context->DrawIndexed(array_count(indices), 0, 0);

    d3d_texture->Release();
}

static void
d3d_draw_cube_texture_instanced(Bitmap* texture){
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

    //----texture buffer----
    {
        D3D11_TEXTURE2D_DESC desc = {
            .Width = (u32)texture->width,
            .Height = (u32)texture->height,
            .MipLevels = 1, // mip levels to use. Set to 0 for mips
            .ArraySize = 1,
            .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
            .SampleDesc = {1, 0},
            .Usage = D3D11_USAGE_IMMUTABLE,
            .BindFlags = D3D11_BIND_SHADER_RESOURCE,
        };

        D3D11_SUBRESOURCE_DATA data = {
            .pSysMem = texture->base,
            .SysMemPitch = (u32)texture->stride,
        };

        hr = d3d_device->CreateTexture2D(&desc, &data, &d3d_texture);
        assert_hr(hr);

        hr = d3d_device->CreateShaderResourceView(d3d_texture, 0, &d3d_shader_resource);
        assert_hr(hr);

        d3d_context->PSSetShaderResources(0, 1, &d3d_shader_resource);
    }

    //----state----
    d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d_context->PSSetSamplers(0, 1, &d3d_sampler_state);
    d3d_context->PSSetShaderResources(0, 1, &d3d_shader_resource);

    d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, d3d_depthbuffer_view);
    d3d_context->OMSetDepthStencilState(d3d_depthstencil_state, 0);
    d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xFFFFFFFF);

    d3d_context->RSSetState(d3d_rasterizer_state);
    d3d_context->RSSetViewports(1, &d3d_viewport);

    d3d_context->IASetInputLayout(d3d_3d_input_layout);
    d3d_context->VSSetShader(d3d_3d_vertex_shader, 0, 0);
    d3d_context->PSSetShader(d3d_3d_pixel_shader, 0, 0);

    d3d_context->DrawIndexedInstanced(array_count(indices), 3, 0, 0, 0);

    d3d_texture->Release();
}

static void
d3d_present(){
    d3d_swapchain->Present(1, 0);
}
