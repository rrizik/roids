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
    f32 x;
    f32 y;
    Rect rect;
    RGBA color;

    Font font;
    String8 text;

	Texture2D* texture;
} RenderCommand;

static void push_clear_color(Arena* arena, RGBA color);
static void push_quad(Arena* arena, Rect rect, RGBA color);
static void push_text(Arena* arena, Font font, f32 x, f32 y, RGBA color, String8 text);
static void push_texture(Arena* arena, ID3D11ShaderResourceView** shader_resource, Rect rect, RGBA color=WHITE);
static void draw_commands(Arena* commands);

static void d3d_clear_color(RGBA color);
static void d3d_draw_text(Font font, f32 x, f32 y, RGBA color, String8 text);
static void d3d_draw_quad(Rect rect, RGBA color);
static void d3d_draw_texture(Texture2D* texture, Rect rect, RGBA color=WHITE);

static void d3d_draw_textured_cube_instanced(ID3D11ShaderResourceView** shader_resource);
static void d3d_present();

#endif
