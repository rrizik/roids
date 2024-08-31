#ifndef D3D11_RENDER_C
#define D3D11_RENDER_C



//u32 count = 0;
static v2
world_from_screen_space(v2 point){
    v2 result = {0};
    result.x = (point.x - (window.width / 2.0f));
    result.y = (window.height / 2.0f - point.y);
    result.y = -result.y;
    return(result);
}

static v2
screen_from_world_space(v2 point){
    v2 result = {0};
    result.x = (point.x + (window.width / 2.0f));
    result.y = ((window.height / 2.0f) + point.y);
    return(result);
}

static void
draw_commands(void){
    begin_timed_function();
    void* at = rc_arena->base;
    void* end = (u8*)rc_arena->base + rc_arena->at;

    // todo: inline most/all of these
    while(at != end){
        RenderCommand* command = (RenderCommand*)at;

        switch(command->type){
            case RenderCommandType_ClearColor:{
                d3d_context->ClearRenderTargetView(d3d_framebuffer_view, command->color.e);
            } break;
            case RenderCommandType_Quad:{
                ID3D11ShaderResourceView* texture = ts->assets.textures[command->texture_id].view;
                RGBA linear_color = srgb_to_linear(command->color); // gamma correction

                v2 p0 = round_v2(command->p0);
                v2 p1 = round_v2(command->p1);
                v2 p2 = round_v2(command->p2);
                v2 p3 = round_v2(command->p3);

                //count += 6;
                Vertex3 vertices[] = {
                    { p0, linear_color, make_v2(0.0f, 0.0f) },
                    { p1, linear_color, make_v2(1.0f, 0.0f) },
                    { p2, linear_color, make_v2(1.0f, 1.0f) },

                    { p0, linear_color, make_v2(0.0f, 0.0f) },
                    { p2, linear_color, make_v2(1.0f, 1.0f) },
                    { p3, linear_color, make_v2(0.0f, 1.0f) },
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
                d3d_context->PSSetShaderResources(0, 1, &texture);

                d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, 0);
                d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xFFFFFFFF);
                d3d_context->RSSetState(d3d_rasterizer_state);

                d3d_context->VSSetConstantBuffers(0, 1, &d3d_constant_buffer);

                d3d_context->RSSetViewports(1, &d3d_viewport);
                d3d_context->IASetInputLayout(d3d_2d_textured_il);
                d3d_context->VSSetShader(d3d_2d_textured_vs, 0, 0);
                d3d_context->PSSetShader(d3d_2d_textured_ps, 0, 0);

                d3d_context->Draw(6, 0);
            } break;
            case RenderCommandType_Line:{
                ID3D11ShaderResourceView* texture = ts->assets.textures[command->texture_id].view;
                RGBA linear_color = srgb_to_linear(command->color); // gamma correction

                v2 p0 = round_v2(command->p0);
                v2 p1 = round_v2(command->p1);
                v2 p2 = round_v2(command->p2);
                v2 p3 = round_v2(command->p3);

                //count += 6;
                Vertex3 vertices[] = {
                    { p0, linear_color, make_v2(0.0f, 0.0f) },
                    { p1, linear_color, make_v2(1.0f, 0.0f) },
                    { p2, linear_color, make_v2(1.0f, 1.0f) },

                    { p0, linear_color, make_v2(0.0f, 0.0f) },
                    { p2, linear_color, make_v2(1.0f, 1.0f) },
                    { p3, linear_color, make_v2(0.0f, 1.0f) },
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
                d3d_context->PSSetShaderResources(0, 1, &texture);

                d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, 0);
                d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xFFFFFFFF);
                d3d_context->RSSetState(d3d_rasterizer_state);

                d3d_context->VSSetConstantBuffers(0, 1, &d3d_constant_buffer);

                d3d_context->RSSetViewports(1, &d3d_viewport);
                d3d_context->IASetInputLayout(d3d_2d_textured_il);
                d3d_context->VSSetShader(d3d_2d_textured_vs, 0, 0);
                d3d_context->PSSetShader(d3d_2d_textured_ps, 0, 0);

                d3d_context->Draw(6, 0);
            } break;
            case RenderCommandType_Texture:{
                ID3D11ShaderResourceView* texture = ts->assets.textures[command->texture_id].view;
                RGBA linear_color = srgb_to_linear(command->color); // gamma correction

                v2 p0 = round_v2(command->p0);
                v2 p1 = round_v2(command->p1);
                v2 p2 = round_v2(command->p2);
                v2 p3 = round_v2(command->p3);

                //count += 6;
                Vertex3 vertices[] = {
                    { p0, linear_color, make_v2(0.0f, 0.0f) },
                    { p1, linear_color, make_v2(1.0f, 0.0f) },
                    { p2, linear_color, make_v2(1.0f, 1.0f) },

                    { p0, linear_color, make_v2(0.0f, 0.0f) },
                    { p2, linear_color, make_v2(1.0f, 1.0f) },
                    { p3, linear_color, make_v2(0.0f, 1.0f) },
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
                d3d_context->PSSetShaderResources(0, 1, &texture);

                d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, 0);
                d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xFFFFFFFF);
                d3d_context->RSSetState(d3d_rasterizer_state);

                d3d_context->VSSetConstantBuffers(0, 1, &d3d_constant_buffer);

                d3d_context->RSSetViewports(1, &d3d_viewport);
                d3d_context->IASetInputLayout(d3d_2d_textured_il);
                d3d_context->VSSetShader(d3d_2d_textured_vs, 0, 0);
                d3d_context->PSSetShader(d3d_2d_textured_ps, 0, 0);

                d3d_context->Draw(6, 0);
            } break;
            case RenderCommandType_Text:{
                Font* font = command->font;
                RGBA linear_color = srgb_to_linear(command->color);

                ScratchArena scratch = begin_scratch();
                u64 allocation_size = command->text.size * 6;
                Vertex3* buffer = push_array(scratch.arena, Vertex3, allocation_size);

                f32 start_x = command->p0.x;
                f32 y_offset = 0;
                stbtt_aligned_quad quad;
                u32 vertex_count = 0;
                for(s32 i=0; i < command->text.size; ++i){
                    u8* character = command->text.str + i;
                    if(*character == '\n'){
                        y_offset += (f32)font->vertical_offset;
                        command->p0.x = start_x;
                    }
                    else{

                        stbtt_GetPackedQuad(font->packed_chars, font->texture_w, font->texture_h, (*character) - font->first_char, &command->p0.x, &command->p0.y, &quad, 1);
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

                        //count += 6;
                        buffer[vertex_count++] = { p0, linear_color, make_v2(quad.s0, quad.t0) };
                        buffer[vertex_count++] = { p1, linear_color, make_v2(quad.s1, quad.t0) };
                        buffer[vertex_count++] = { p2, linear_color, make_v2(quad.s1, quad.t1) };

                        buffer[vertex_count++] = { p0, linear_color, make_v2(quad.s0, quad.t0) };
                        buffer[vertex_count++] = { p2, linear_color, make_v2(quad.s1, quad.t1) };
                        buffer[vertex_count++] = { p3, linear_color, make_v2(quad.s0, quad.t1) };
                    }
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
                d3d_context->PSSetShaderResources(0, 1, &font->texture.view);

                d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, 0);
                d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xFFFFFFFF);
                d3d_context->RSSetState(d3d_rasterizer_state);

                d3d_context->RSSetViewports(1, &d3d_viewport);
                d3d_context->IASetInputLayout(d3d_2d_textured_il);
                d3d_context->VSSetShader(d3d_2d_textured_vs, 0, 0);
                d3d_context->PSSetShader(d3d_2d_textured_ps, 0, 0);

                d3d_context->Draw((UINT)vertex_count, 0);
                end_scratch(scratch);
            } break;
        }
		at = (u8*)at + sizeof(RenderCommand);
    }
    //print("vertex_count: %i, vertex_bytes: %i, vertex_mb: %i\n", count, count * sizeof(Vertex3), (count * sizeof(Vertex3)) / MB(1));
    //count = 0;
    {
        begin_timed_scope("present");
        d3d_swapchain->Present(1, 0);
    }
}

#endif
