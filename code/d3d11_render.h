#pragma once

static void
d3d_clear_color(RGBA color){
    d3d_context->ClearRenderTargetView(d3d_framebuffer_view, color.e);
    d3d_context->ClearDepthStencilView(d3d_depthbuffer_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

static void
d3d_draw_cube(Mesh* mesh, Bitmap texture, v3 pos, v3 angle, v3 scale){

    d3d_set_vertex_buffer(mesh, cube);
    d3d_set_index_buffer(mesh, cube_indicies);
    d3d_set_constant_buffer(pos, angle, scale);
    d3d_set_texture(texture);

    d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d_context->PSSetSamplers(0, 1, &d3d_sampler_state);
    d3d_context->PSSetShaderResources(0, 1, &d3d_shader_resource);

    d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, d3d_depthbuffer_view);
    d3d_context->OMSetDepthStencilState(d3d_depthstencil_state, 0);
    d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xAFFFFFFF);

    d3d_context->RSSetState(d3d_rasterizer_state);
    d3d_context->RSSetViewports(1, &d3d_viewport);

    d3d_context->DrawIndexed(mesh->index_count, 0, 0);
    //d3d_context->DrawIndexedInstanced(mesh->index_count, 0, 0, 0, 0);
}