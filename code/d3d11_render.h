#pragma once

static void
d3d_clear_color(RGBA color){
    d3d_context->ClearRenderTargetView(d3d_framebuffer_view, color.e);
    d3d_context->ClearDepthStencilView(d3d_depthbuffer_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

static void
d3d_draw_cube_indexed(Mesh* mesh, Bitmap* texture, v3 pos, v3 angle, v3 scale){
    D3D11_BUFFER_DESC vertex_buffer_desc = {0};
    vertex_buffer_desc.StructureByteStride = mesh->vertex_stride;
    vertex_buffer_desc.ByteWidth = mesh->vertex_stride * mesh->vertex_count;
    vertex_buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertex_resource = {0};
    vertex_resource.pSysMem = mesh->verticies;
    hr = d3d_device->CreateBuffer(&vertex_buffer_desc, &vertex_resource, &mesh->vertex_buffer);
    assert_hr(hr);

    ID3D11Buffer* buffers[] = {mesh->vertex_buffer};
    u32 strides[] = {sizeof(Vertex)};
    u32 offset[] = {0, 0};

    d3d_context->IASetVertexBuffers(0, 2, buffers, strides, offset);
    //d3d_set_vertex_buffer(mesh, cube);
    //-------------------------------------------------------------------

    D3D11_BUFFER_DESC index_buffer_desc = {0};

    index_buffer_desc.StructureByteStride = mesh->index_stride;
    index_buffer_desc.ByteWidth = mesh->index_stride * mesh->index_count;
    index_buffer_desc.Usage     = D3D11_USAGE_DEFAULT;
    index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA index_resource  = {0};
    index_resource.pSysMem                 = mesh->indicies;
    hr = d3d_device->CreateBuffer(&index_buffer_desc, &index_resource, &mesh->index_buffer);
    assert_hr(hr);

    d3d_context->IASetIndexBuffer(mesh->index_buffer, DXGI_FORMAT_R32_UINT, 0);
    //d3d_set_index_buffer(mesh, cube_indicies);
    //-------------------------------------------------------------------

    //d3d_set_constant_buffer(pos, angle, scale);

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

    hr = d3d_device->CreateTexture2D(&texture_desc, &data, &d3d_texture);
    assert_hr(hr);

    hr = d3d_device->CreateShaderResourceView(d3d_texture, 0, &d3d_shader_resource);
    assert_hr(hr);

    d3d_context->PSSetShaderResources(0, 1, &d3d_shader_resource);
    //d3d_set_texture(texture);
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

    d3d_context->DrawIndexed(mesh->index_count, 0, 0);
}

static void
d3d_draw_cube_instanced(Mesh* mesh, Bitmap* texture){
    //d3d_set_constant_buffer(pos, angle, scale);
    //deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);

    D3D11_BUFFER_DESC vertex_buffer_desc = {0};
    vertex_buffer_desc.StructureByteStride = mesh->vertex_stride;
    vertex_buffer_desc.ByteWidth = mesh->vertex_stride * mesh->vertex_count;
    vertex_buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertex_resource = {0};
    vertex_resource.pSysMem = mesh->verticies;
    hr = d3d_device->CreateBuffer(&vertex_buffer_desc, &vertex_resource, &mesh->vertex_buffer);
    assert_hr(hr);

    ID3D11Buffer* buffers[] = {mesh->vertex_buffer, d3d_instance_buffer};
    u32 strides[] = {sizeof(Vertex), sizeof(InstanceData)};
    u32 offset[] = {0, 0};

    d3d_context->IASetVertexBuffers(0, 2, buffers, strides, offset);
    //d3d_set_vertex_buffer(mesh, cube);
    //-------------------------------------------------------------------

    D3D11_BUFFER_DESC index_buffer_desc = {0};

    index_buffer_desc.StructureByteStride = mesh->index_stride;
    index_buffer_desc.ByteWidth = mesh->index_stride * mesh->index_count;
    index_buffer_desc.Usage     = D3D11_USAGE_DEFAULT;
    index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA index_resource  = {0};
    index_resource.pSysMem                 = mesh->indicies;
    hr = d3d_device->CreateBuffer(&index_buffer_desc, &index_resource, &mesh->index_buffer);
    assert_hr(hr);

    d3d_context->IASetIndexBuffer(mesh->index_buffer, DXGI_FORMAT_R32_UINT, 0);
    //d3d_set_index_buffer(mesh, cube_indicies);
    //-------------------------------------------------------------------

    D3D11_BUFFER_DESC instance_buffer_desc = {0};
    instance_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
    instance_buffer_desc.ByteWidth = sizeof(InstanceData) * instance_count;
    instance_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA instance_data = {};
    instance_data.pSysMem = {cube_instances};

    hr = d3d_device->CreateBuffer(&instance_buffer_desc, &instance_data, &d3d_instance_buffer);
    assert_hr(hr);
    //d3d_init_instance_buffer(instances);
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

    hr = d3d_device->CreateTexture2D(&texture_desc, &data, &d3d_texture);
    assert_hr(hr);

    hr = d3d_device->CreateShaderResourceView(d3d_texture, 0, &d3d_shader_resource);
    assert_hr(hr);

    d3d_context->PSSetShaderResources(0, 1, &d3d_shader_resource);
    //d3d_set_texture(texture);
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

    //deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
    d3d_context->DrawIndexedInstanced(mesh->index_count, 2, 0, 0, 0);
}
