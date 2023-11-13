#pragma once

static void
d3d_clear_color(RGBA color){
    d3d_context->ClearRenderTargetView(d3d_framebuffer_view, color.e);
    d3d_context->ClearDepthStencilView(d3d_depthbuffer_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

static void
d3d_draw_quad(f32 x0, f32 y0, f32 x1, f32 y1, RGBA color){
    d3d_load_shader(str8_literal("2d_shader.hlsl"), d3d_2dui_color_input_layout, 2);

    Vertex vertices[] = {
        { make_v3(x0, y0,  0.0f), color, make_v2(0.0f, 0.0f) },
        { make_v3(x1, y0,  0.0f), color, make_v2(0.0f, 0.0f) },
        { make_v3(x0, y1,  0.0f), color, make_v2(0.0f, 0.0f) },
        { make_v3(x1, y1,  0.0f), color, make_v2(0.0f, 0.0f) },
    };

    s32 indices[] = {
        2, 3, 0, // top left
        0, 3, 1, // bottom right
    };

    D3D11_BUFFER_DESC vertex_buffer_desc = {0};
    vertex_buffer_desc.StructureByteStride = sizeof(Vertex);
    vertex_buffer_desc.ByteWidth = sizeof(Vertex) * array_count(vertices);
    vertex_buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertex_resource = {0};
    vertex_resource.pSysMem = vertices;
    ID3D11Buffer* vertex_buffer;
    hr = d3d_device->CreateBuffer(&vertex_buffer_desc, &vertex_resource, &vertex_buffer);
    assert_hr(hr);

    ID3D11Buffer* buffers[] = {vertex_buffer};
    u32 strides[] = {sizeof(Vertex)};
    u32 offset[] = {0};

    d3d_context->IASetVertexBuffers(0, 1, buffers, strides, offset);
    //-------------------------------------------------------------------

    D3D11_BUFFER_DESC index_buffer_desc = {0};

    index_buffer_desc.StructureByteStride = sizeof(u32);
    index_buffer_desc.ByteWidth = sizeof(u32) * array_count(indices);
    index_buffer_desc.Usage     = D3D11_USAGE_DEFAULT;
    index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA index_resource  = {0};
    index_resource.pSysMem                 = indices;
    ID3D11Buffer* index_buffer;
    hr = d3d_device->CreateBuffer(&index_buffer_desc, &index_resource, &index_buffer);
    assert_hr(hr);

    d3d_context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);
    //-------------------------------------------------------------------

    d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d_context->PSSetSamplers(0, 1, &d3d_sampler_state);
    d3d_context->PSSetShaderResources(0, 1, &d3d_shader_resource);

    d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, d3d_depthbuffer_view);
    d3d_context->OMSetDepthStencilState(d3d_depthstencil_state, 0);
    d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xFFFFFFFF);

    d3d_context->RSSetState(d3d_rasterizer_state);
    d3d_context->RSSetViewports(1, &d3d_viewport);
    d3d_context->IASetInputLayout(d3d_input_layout);

    d3d_context->VSSetShader(d3d_vertex_shader, 0, 0);
    d3d_context->PSSetShader(d3d_pixel_shader, 0, 0);

    d3d_context->DrawIndexed(array_count(indices), 0, 0);
}

static void
d3d_draw_quad_texture(f32 x0, f32 y0, f32 x1, f32 y1, Bitmap* texture){
    d3d_load_shader(str8_literal("2d_texture_shader.hlsl"), d3d_2dui_texture_input_layout, 3);

    Vertex vertices[] = {
        { make_v3(x0, y0, 0.0f), WHITE, make_v2(0.0f, 0.0f)},
        { make_v3(x1, y0, 0.0f), WHITE, make_v2(0.0f, 1.0f)},
        { make_v3(x0, y1, 0.0f), WHITE, make_v2(1.0f, 0.0f)},
        { make_v3(x1, y1, 0.0f), WHITE, make_v2(1.0f, 1.0f)},
    };

    s32 indices[] = {
        2, 3, 0, // top left
        0, 3, 1, // bottom right
    };

    D3D11_BUFFER_DESC vertex_buffer_desc = {0};
    vertex_buffer_desc.StructureByteStride = sizeof(Vertex);
    vertex_buffer_desc.ByteWidth = sizeof(Vertex) * array_count(vertices);
    vertex_buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertex_resource = {0};
    vertex_resource.pSysMem = vertices;
    ID3D11Buffer* vertex_buffer;
    hr = d3d_device->CreateBuffer(&vertex_buffer_desc, &vertex_resource, &vertex_buffer);
    assert_hr(hr);

    ID3D11Buffer* buffers[] = {vertex_buffer};
    u32 strides[] = {sizeof(Vertex)};
    u32 offset[] = {0};

    d3d_context->IASetVertexBuffers(0, 1, buffers, strides, offset);
    //-------------------------------------------------------------------

    D3D11_BUFFER_DESC index_buffer_desc = {0};

    index_buffer_desc.StructureByteStride = sizeof(u32);
    index_buffer_desc.ByteWidth = sizeof(u32) * array_count(indices);
    index_buffer_desc.Usage     = D3D11_USAGE_DEFAULT;
    index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA index_resource  = {0};
    index_resource.pSysMem                 = indices;
    ID3D11Buffer* index_buffer;
    hr = d3d_device->CreateBuffer(&index_buffer_desc, &index_resource, &index_buffer);
    assert_hr(hr);

    d3d_context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);
    //-------------------------------------------------------------------

    D3D11_TEXTURE2D_DESC texture_desc = {
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

    ID3D11Texture2D* d3d_texture;
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
    d3d_context->IASetInputLayout(d3d_input_layout);

    d3d_context->VSSetShader(d3d_vertex_shader, 0, 0);
    d3d_context->PSSetShader(d3d_pixel_shader, 0, 0);

    d3d_context->DrawIndexed(array_count(indices), 0, 0);
}

// TODO: definition needs to change once we establish the different ways to draw things
static void
d3d_draw_cube_texture_instanced(Bitmap* texture){

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

    d3d_load_shader(str8_literal("3d_shader.hlsl"), d3d_3d_input_layout, 7);

    D3D11_BUFFER_DESC instance_buffer_desc = {0};
    instance_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
    instance_buffer_desc.ByteWidth = sizeof(InstanceData) * instance_count;
    instance_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA instance_data = {};
    instance_data.pSysMem = {cube_instances};

    ID3D11Buffer* instance_buffer;
    hr = d3d_device->CreateBuffer(&instance_buffer_desc, &instance_data, &instance_buffer);
    assert_hr(hr);
    //-------------------------------------------------------------------

    D3D11_BUFFER_DESC vertex_buffer_desc = {0};
    vertex_buffer_desc.StructureByteStride = sizeof(Vertex);
    vertex_buffer_desc.ByteWidth = sizeof(Vertex) * array_count(vertices);
    vertex_buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertex_resource = {0};
    vertex_resource.pSysMem = vertices;
    ID3D11Buffer* vertex_buffer;
    hr = d3d_device->CreateBuffer(&vertex_buffer_desc, &vertex_resource, &vertex_buffer);
    assert_hr(hr);

    ID3D11Buffer* buffers[] = {vertex_buffer, instance_buffer};
    u32 strides[] = {sizeof(Vertex), sizeof(InstanceData)};
    u32 offset[] = {0, 0};

    d3d_context->IASetVertexBuffers(0, 2, buffers, strides, offset);
    //-------------------------------------------------------------------

    D3D11_BUFFER_DESC index_buffer_desc = {0};

    index_buffer_desc.StructureByteStride = sizeof(u32);
    index_buffer_desc.ByteWidth = sizeof(u32) * array_count(indices);
    index_buffer_desc.Usage     = D3D11_USAGE_DEFAULT;
    index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA index_resource  = {0};
    index_resource.pSysMem                 = indices;
    ID3D11Buffer* index_buffer;
    hr = d3d_device->CreateBuffer(&index_buffer_desc, &index_resource, &index_buffer);
    assert_hr(hr);

    d3d_context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);
    //-------------------------------------------------------------------

    D3D11_TEXTURE2D_DESC texture_desc = {
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

    ID3D11Texture2D* d3d_texture;
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
    d3d_context->IASetInputLayout(d3d_input_layout);

    d3d_context->VSSetShader(d3d_vertex_shader, 0, 0);
    d3d_context->PSSetShader(d3d_pixel_shader, 0, 0);

    d3d_context->DrawIndexedInstanced(array_count(indices), 3, 0, 0, 0);
}

static void
d3d_present(){
    d3d_swapchain->Present(1, 0);
}
