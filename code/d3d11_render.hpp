#ifndef D3D11_RENDER_H
#define D3D11_RENDER_H

static RGBA srgb_to_linear_approx(RGBA value);
static RGBA linear_to_srgb_approx(RGBA value);
static RGBA srgb_to_linear(RGBA value);

static void d3d_clear_color(RGBA color);
static void d3d_draw_text(Font font, f32 x, f32 y, RGBA color, String8 text);
static void d3d_draw_quad(Rect rect, RGBA color);
static void d3d_draw_quad_textured(Rect rect, RGBA color, ID3D11ShaderResourceView** shader_resource);
static void d3d_draw_textured_cube_instanced(ID3D11ShaderResourceView** shader_resource);
static void d3d_present();

#endif
