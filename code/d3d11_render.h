#pragma once

static void
d3d_clear_color(RGBA color){
    d3d_context->ClearRenderTargetView(d3d_framebuffer_view, color.e);
    d3d_context->ClearDepthStencilView(d3d_depthbuffer_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

static void
d3d_draw_cube_indexed(Mesh* mesh, Bitmap texture, v3 pos, v3 angle, v3 scale){

    //d3d_set_vertex_buffer(mesh, cube);
    d3d_context->IASetVertexBuffers(0, 1, &mesh->vertex_buffer, &mesh->vertex_stride, &mesh->vertex_offset);
    d3d_set_index_buffer(mesh, cube_indicies);
    d3d_set_constant_buffer(pos, angle, scale);
    d3d_set_texture(texture);

    d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d_context->PSSetSamplers(0, 1, &d3d_sampler_state);
    d3d_context->PSSetShaderResources(0, 1, &d3d_shader_resource);

    d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, d3d_depthbuffer_view);
    d3d_context->OMSetDepthStencilState(d3d_depthstencil_state, 0);
    d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xFFFFFFFF);

    d3d_set_viewport(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.1f);
    d3d_context->RSSetState(d3d_rasterizer_state);

    d3d_context->DrawIndexed(mesh->index_count, 0, 0);
}

static void
d3d_draw_cube_instanced(Mesh* mesh, Bitmap texture){
    //d3d_set_vertex_buffer(mesh, cube);
    HRESULT hr;

    D3D11_BUFFER_DESC buffer_desc = {0};
    buffer_desc.StructureByteStride = mesh->vertex_stride;
    buffer_desc.ByteWidth = mesh->vertex_stride * mesh->vertex_count;
    buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertex_resource = {0};
    vertex_resource.pSysMem = cube;
    hr = d3d_device->CreateBuffer(&buffer_desc, &vertex_resource, &mesh->vertex_buffer);
    assert_hr(hr);

    ID3D11Buffer* buffers[] = {mesh->vertex_buffer, d3d_instance_buffer};
    u32 strides[] = {sizeof(Vertex), sizeof(InstanceData)};
    u32 offset[] = {0, 0};
    d3d_context->IASetVertexBuffers(0, 2, buffers, strides, offset);

    d3d_set_index_buffer(mesh, cube_indicies);
    //d3d_set_constant_buffer(pos, angle, scale); //TODO: this information needs to change as it shouldnt be entity position anymore
    d3d_set_instance_buffer(instances);
    d3d_set_texture(texture);

    d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d_context->PSSetSamplers(0, 1, &d3d_sampler_state);
    d3d_context->PSSetShaderResources(0, 1, &d3d_shader_resource);


    d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, d3d_depthbuffer_view);
    d3d_context->OMSetDepthStencilState(d3d_depthstencil_state, 0);
    d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xFFFFFFFF);

    d3d_set_viewport(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.1f);
    d3d_context->RSSetState(d3d_rasterizer_state);

    d3d_context->DrawIndexedInstanced(mesh->index_count, array_count(instances), 0, 0, 0);
}
