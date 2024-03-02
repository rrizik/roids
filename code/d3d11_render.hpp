#ifndef D3D11_RENDER_H
#define D3D11_RENDER_H

static RGBA srgb_to_linear_approx(RGBA value);
static RGBA linear_to_srgb_approx(RGBA value);
static RGBA srgb_to_linear(RGBA value);

Arena* render_command_arena = make_arena(MB(4));

typedef enum RenderCommandType{
    RenderCommandType_ClearColor,
    RenderCommandType_Quad,
    RenderCommandType_Texture,
    RenderCommandType_Text,
} RenderCommandType;

typedef struct RenderCommand{
    RenderCommandType type;
    v2 p0;
    v2 p1;
    v2 p2;
    v2 p3;

    RGBA color;
	ID3D11ShaderResourceView** texture;

    // todo: remove this
    f32 x;
    f32 y;

    Font font;
    String8 text;

} RenderCommand;

static void push_clear_color(Arena* arena, RGBA color);
static void push_quad(Arena* arena, v2 p0, v2 p1, v2 p2, v2 p3, RGBA color);
static void push_text(Arena* arena, Font font, String8 text, f32 x, f32 y, RGBA color);
static void push_texture(Arena* arena, v2 p0, v2 p1, v2 p2, v2 p3, RGBA color, ID3D11ShaderResourceView** texture);
static void draw_commands(Arena* commands);

static void d3d_clear_color(RGBA color);
static void d3d_draw_text(Font font, f32 x, f32 y, RGBA color, String8 text);
static void d3d_draw_quad(v2 p0, v2 p1, v2 p2, v2 p3, RGBA color);
static void d3d_draw_texture(v2 p0, v2 p1, v2 p2, v2 p3, RGBA color, ID3D11ShaderResourceView** texture);

static void d3d_present();

#endif
