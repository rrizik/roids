#ifndef D3D11_RENDER_H
#define D3D11_RENDER_H

static void d3d_clear_color(RGBA color);

static void d3d_draw_text(Font font, f32 x, f32 y, RGBA color, String8 text);
static void d3d_draw_quad(f32 x0, f32 y0, f32 x1, f32 y1, RGBA color);
static void d3d_draw_quad_textured(f32 x0, f32 y0, f32 x1, f32 y1, RGBA color, ID3D11ShaderResourceView** shader_resource);
static void d3d_draw_textured_cube_instanced(ID3D11ShaderResourceView** shader_resource);

static void d3d_present();

#endif
