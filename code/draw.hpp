#ifndef DRAW_H
#define DRAW_H

static RGBA CLEAR =   {1.0f, 1.0f, 1.0f,  0.0f};
static RGBA RED =     {1.0f, 0.0f, 0.0f,  1.0f};
static RGBA GREEN =   {0.0f, 1.0f, 0.0f,  1.0f};
static RGBA BLUE =    {0.0f, 0.0f, 1.0f,  1.0f};
static RGBA MAGENTA = {1.0f, 0.0f, 1.0f,  1.0f};
static RGBA TEAL =    {0.0f, 1.0f, 1.0f,  1.0f};
static RGBA PINK =    {0.92f, 0.62f, 0.96f, 1.0f};
static RGBA YELLOW =  {0.9f, 0.9f, 0.0f,  1.0f};
static RGBA ORANGE =  {1.0f, 0.5f, 0.15f,  1.0f};
static RGBA ORANGE_HALF =  {1.0f, 0.5f, 0.15f,  0.1f};
static RGBA DARK_GRAY =  {0.5f, 0.5f, 0.5f,  1.0f};
static RGBA LIGHT_GRAY = {0.8f, 0.8f, 0.8f,  1.0f};
static RGBA WHITE =   {1.0f, 1.0f, 1.0f,  1.0f};
static RGBA BLACK =   {0.0f, 0.0f, 0.0f,  1.0f};
static RGBA BACKGROUND_COLOR =   {1.0f/255.0f, 1.0f/255.0f, 1.0f/255.0f};
static RGBA ARMY_GREEN =   {0.25f, 0.25f, 0.23f,  1.0f};

global Arena* rb_arena = 0;
global Assets* r_assets = 0;
global Texture* r_texture;

#define DEFAULT_BATCH_SIZE KB(200)
//#define DEFAULT_BATCH_SIZE KB(100)
typedef struct RenderBatch{
    RenderBatch* next;
    Vertex3* buffer;
    s32 id;

    s32 count;
    s32 cap;
    Texture* texture;
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

static v2 screen_from_world(v2 world_pos, Camera2D* camera, Window* window);
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


#endif

