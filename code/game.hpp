#ifndef GAME_H
#define GAME_H

static Font global_font = {0};

typedef enum GameMode{
    GameMode_FirstPerson,
    GameMode_Editor,
    GameMode_Game,
} GameMode;

typedef enum AssetID{
    AssetID_Image,
    AssetID_Ship,
    AssetID_Tree,
    AssetID_Circle,
    AssetID_Bullet,
    AssetID_Test,

    AssetID_Count,
} AssetID;

typedef struct Assets{
    Bitmap bitmaps[AssetID_Count];
    //ID3D11Texture2D* textures[AssetID_Count];
    //ID3D11ShaderResourceView* shader_resources[AssetID_Count];
} Assets;

static void load_assets(Arena* arena, Assets* assets);

//static void load_textures_from_assets(Assets* assets);

static Bitmap* get_bitmap(Assets* assets, AssetID id);

//static ID3D11ShaderResourceView* get_shader_resource(Assets* assets, AssetID id);

#define ENTITIES_MAX 100
typedef struct PermanentMemory{
    Arena arena;
    u32 game_mode;

    u32 generation[ENTITIES_MAX];
    u32 free_entities[ENTITIES_MAX];
    u32 free_entities_at;

    Entity entities[ENTITIES_MAX];
    u32 entities_count;

    Entity* texture;
    Entity* circle;
    Entity* basis;
    Entity* ship;
    Bitmap tree;
    bool ship_loaded;

} PermanentMemory, State;
global PermanentMemory* pm;

typedef struct TransientMemory{
    Arena arena;
    Arena *frame_arena;
    Arena *render_command_arena;

    Assets assets;
} TransientMemory;
global TransientMemory* tm;

static Entity* entity_from_handle(PermanentMemory* pm, EntityHandle handle);
static EntityHandle handle_from_entity(PermanentMemory* pm, Entity *e);
static void remove_entity(PermanentMemory* pm, Entity* e);
static Entity* add_entity(PermanentMemory *pm, EntityType type);
static Entity* add_pixel(PermanentMemory* pm, Rect rect, RGBA color);
static Entity* add_segment(PermanentMemory* pm, v2 p0, v2 p1, RGBA color);
static Entity* add_ray(PermanentMemory* pm, Rect rect, v2 direction, RGBA color);
static Entity* add_line(PermanentMemory* pm, Rect rect, v2 direction, RGBA color);
//static Entity* add_rect(PermanentMemory* pm, Rect rect, RGBA color, s32 bsize = 0, RGBA bcolor = {0, 0, 0, 0});
static Entity* add_basis(PermanentMemory* pm, v2 origin, v2 x_axis, v2 y_axis, Bitmap* texture, RGBA color = {0, 0, 0, 1});
static Entity* add_ship(PermanentMemory* pm, v2 origin, v2 x_axis, v2 y_axis, Bitmap* texture, RGBA color = {0, 0, 0, 1});
static Entity* add_bullet(PermanentMemory* pm, v2 origin, v2 x_axis, v2 y_axis, Bitmap* texture, RGBA color = {0, 0, 0, 1});
static Entity* add_cube(PermanentMemory* pm, Bitmap* texture, v3 pos, v3 angle, v3 scale, u32 index);
static Entity* add_player(PermanentMemory* pm, Bitmap* texture, v3 pos, v3 angle, v3 scale, u32 index);
static Entity* add_box(PermanentMemory* pm, Rect rect, RGBA color);
static Entity* add_quad(PermanentMemory* pm, v2 p0, v2 p1, v2 p2, v2 p3, RGBA color, bool fill);
static Entity* add_triangle(PermanentMemory *pm, v2 p0, v2 p1, v2 p2, RGBA color, bool fill);
static Entity* add_circle(PermanentMemory *pm, Rect rect, u8 rad, RGBA color, bool fill);
static Entity* add_bitmap(PermanentMemory* pm, v2 pos, Bitmap* texture);
static void entities_clear(PermanentMemory* pm);
static void serialize_data(PermanentMemory* pm, String8 filename);
static void deserialize_data(PermanentMemory* pm, String8 filename);
static bool handle_global_events(Event event);
static bool handle_camera_events(Event event);
static bool handle_controller_events(Event event);

//static String8 tree_str   = str8_literal("tree00.bmp");
//static String8 image_str  = str8_literal("image.bmp");
//static String8 test_str   = str8_literal("test3.bmp");
//static String8 circle_str = str8_literal("circle.bmp");
//static String8 ship_str   = str8_literal("ship_simple.bmp");
//static String8 bullet_str = str8_literal("bullet4.bmp");

static f32 angle = 0;
static void update_game(Window* window, Memory* memory, Events* events, Clock* clock);

#endif

