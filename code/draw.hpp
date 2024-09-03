#ifndef DRAW_H
#define DRAW_H

global Arena* rb_arena = 0;
global Assets* r_assets = 0;
global Texture* r_texture;

#define DEFAULT_BATCH_SIZE KB(200)
//#define DEFAULT_BATCH_SIZE KB(100)
typedef struct RenderBatch{
    RenderBatch* next;
    Vertex3* buffer;
    s32 count;
    s32 cap;
    Texture* texture;
    s32 id;
} RenderBatch;

typedef struct RenderBatchNode{
    RenderBatch* first;
    RenderBatch* last;
    s32 count;
} RenderBatchNode;
global RenderBatchNode render_batches = {0};

static void set_texture(Texture* texture);
static Texture* get_texture(void);
static RenderBatch* get_render_batch(u64 vertex_count);

typedef enum RenderCommandType{
    RenderCommandType_ClearColor,
    RenderCommandType_Quad,
    RenderCommandType_Line,
    RenderCommandType_Texture,
    RenderCommandType_Text,
} RenderCommandType;

typedef struct RenderCommand{
    RenderCommandType type;

    union {
        v2 pos;
        v2 p0;
    };
    v2 p1;
    v2 p2;
    v2 p3;

    RGBA color;
    Font* font;
	u32 texture_id;

    String8 text;
} RenderCommand;

static RGBA srgb_to_linear_approx(RGBA value);
static RGBA linear_to_srgb_approx(RGBA value);
static RGBA srgb_to_linear(RGBA value);

static void init_render_commands(Arena* arena);

static void draw_quad(v2 p0, v2 p1, v2 p2, v2 p3, RGBA color);
static void draw_quad(v2 pos, v2 dim, RGBA color);
static void draw_quad(Rect rect, RGBA color);
static void draw_quad(Quad quad, RGBA color);

// todo: not implemented yet
static void draw_quad(v2 p0, v2 p1, v2 p2, v2 p3, v2 uv0, v2 uv1, v2 uv2, v2 uv3, RGBA color);
static void draw_quad(v2 pos, v2 dim, v2 uv0, v2 uv1, v2 uv2, v2 uv3, RGBA color);
static void draw_quad(Quad quad, v2 uv0, v2 uv1, v2 uv2, v2 uv3, RGBA color);
static void draw_quad(Rect rect, v2 uv0, v2 uv1, v2 uv2, v2 uv3, RGBA color);

static void draw_line(v2 p0, v2 p1, f32 width, RGBA color);
static void draw_text(Font* font, String8 text, v2 pos, RGBA color);
static void draw_texture(u32 texture, v2 p0, v2 p1, v2 p2, v2 p3, RGBA color=WHITE);

static void draw_render_batches(void);
static void render_batches_reset(void);

static v2 screen_from_world(v2 world_pos, v2 origin, float scale){
    v2 result = {0};
    result.x = (world_pos.x - origin.x) * scale;
    result.y = (world_pos.y - origin.y) * scale;
    //result.x = (origin.x - world_pos.x) * scale;
    //result.y = (origin.y - world_pos.y) * scale;
    return(result);
}

static v2
screen_from_world_space(v2 world_pos, v2 origin, f32 scale){
    v2 result = {0};
    result.x = (world_pos.x + origin.x) * scale;
    result.y = (origin.y + world_pos.y) * scale;
    return(result);
}

static v2
scaled_dim(v2 size, f32 scale){
    v2 result = {0};
    result.x = size.x * scale;
    result.y = size.y * scale;
    return(result);
}

#endif
