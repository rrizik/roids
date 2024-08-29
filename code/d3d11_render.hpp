#ifndef D3D11_RENDER_H
#define D3D11_RENDER_H

static RGBA srgb_to_linear_approx(RGBA value);
static RGBA linear_to_srgb_approx(RGBA value);
static RGBA srgb_to_linear(RGBA value);

typedef enum RenderCommandType{
    RenderCommandType_ClearColor,
    RenderCommandType_Quad,
    RenderCommandType_Line,
    RenderCommandType_Texture,
    RenderCommandType_Text,
} RenderCommandType;

typedef enum RenderCommandSpace{
    RenderCommandSpace_Screen,
    RenderCommandSpace_World,
} RenderCommandSpace;

typedef struct RenderCommand{
    RenderCommandType type;
    RenderCommandSpace space;

    v2 p0;
    v2 p1;
    v2 p2;
    v2 p3;

    s32 width;

    RGBA color;
	u32 texture;

    u32 font_id;
    String8 text;
} RenderCommand;

s32 count = 0;
Arena* rc_arena = 0;

static void init_render_commands(Arena* arena);
static void draw_clear_color(Arena* arena, RGBA color);

static void draw_quad(v2 p0, v2 p1, v2 p2, v2 p3, RGBA color);
static void draw_quad(v2 pos, v2 dim, RGBA color);
static void draw_quad(Quad quad, RGBA color);
static void draw_quad(Rect rect, RGBA color);
// todo: not implemented yet
static void draw_quad(v2 p0, v2 p1, v2 p2, v2 p3, v2 uv0, v2 uv1, v2 uv2, v2 uv3, RGBA color);
static void draw_quad(v2 pos, v2 dim, v2 uv0, v2 uv1, v2 uv2, v2 uv3, RGBA color);
static void draw_quad(Quad quad, v2 uv0, v2 uv1, v2 uv2, v2 uv3, RGBA color);
static void draw_quad(Rect rect, v2 uv0, v2 uv1, v2 uv2, v2 uv3, RGBA color);

static void draw_line(v2 p0, v2 p1, f32 width, RGBA color);
static void draw_text(u32 font_id, String8 text, v2 pos, RGBA color);
static void draw_texture(v2 p0, v2 p1, v2 p2, v2 p3, RGBA color, ID3D11ShaderResourceView** texture);
static void draw_commands();

#endif
