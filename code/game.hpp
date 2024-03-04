#ifndef GAME_H
#define GAME_H

typedef enum GameMode{
    GameMode_FirstPerson,
    GameMode_Editor,
    GameMode_Game,
} GameMode;

static void load_assets(Arena* arena, Assets* assets);

static Bitmap* get_bitmap(Assets* assets, AssetID id);

static Entity* entity_from_handle(PermanentMemory* pm, EntityHandle handle);
static EntityHandle handle_from_entity(PermanentMemory* pm, Entity *e);

static void    remove_entity(PermanentMemory* pm, Entity* e);
static Entity* add_entity(PermanentMemory *pm, EntityType type);
static Entity* add_quad(PermanentMemory* pm, v2 pos, v2 dim, RGBA color);
static Entity* add_texture(PermanentMemory* pm, ID3D11ShaderResourceView** texture, v2 pos, v2 dim, RGBA color=WHITE);

static Entity* add_pixel(PermanentMemory* pm, RGBA color);
static Entity* add_segment(PermanentMemory* pm, v2 p0, v2 p1, RGBA color);
static Entity* add_ray(PermanentMemory* pm, v2 direction, RGBA color);
static Entity* add_line(PermanentMemory* pm, v2 direction, RGBA color);

static Entity* add_ship(PermanentMemory* pm, v2 origin, v2 x_axis, v2 y_axis, Bitmap* texture, RGBA color = {0, 0, 0, 1});
static Entity* add_bullet(PermanentMemory* pm, v2 origin, v2 x_axis, v2 y_axis, Bitmap* texture, RGBA color = {0, 0, 0, 1});
static Entity* add_player(PermanentMemory* pm, Bitmap* texture, v2 pos, f32 angle, v2 scale, u32 index);
static Entity* add_quad(PermanentMemory* pm, v2 p0, v2 p1, v2 p2, v2 p3, RGBA color, bool fill);


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

static void update_game(Window* window, Memory* memory, Events* events);

static v3 angle1 = make_v3(0, 0, 0);

#endif

