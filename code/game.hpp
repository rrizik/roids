#ifndef GAME_H
#define GAME_H

typedef enum WaveAsset{
    WaveAsset_track1,
    WaveAsset_track2,
    WaveAsset_track3,
    WaveAsset_track4,
    WaveAsset_track5,
    WaveAsset_rail1,
    WaveAsset_rail2,
    WaveAsset_rail3,
    WaveAsset_rail4,
    WaveAsset_rail5,

    WaveAsset_Count,
} WaveAsset;

typedef enum TextureAsset{
    TextureAsset_Ship,
    TextureAsset_Bullet,
    TextureAsset_Asteroid,

    TextureAsset_Font_Arial,
    TextureAsset_Font_Golos,

    TextureAsset_Count,
} TextureAsset;

typedef enum FontAsset{
    FontAsset_Arial,
    FontAsset_Golos,
    FontAsset_Consolas,

    FontAsset_Count,
} FontAsset;

typedef struct Assets{
    Wave    waves[WaveAsset_Count];
    Font    fonts[FontAsset_Count];
    Texture textures[TextureAsset_Count];
} Assets;

typedef struct Level{
    s32 asteroid_spawned;
    s32 asteroid_destroyed;
    s32 asteroid_count_max;
} Level;

static void init_levels();

typedef enum GameMode{
    GameMode_FirstPerson,
    GameMode_Editor,
    GameMode_Game,
} GameMode;

static void load_assets(Arena* arena, Assets* assets);

//static Bitmap* get_bitmap(Assets* assets, BitmapAsset id);

// todo: is global, stop passing it around
static Entity* entity_from_handle(EntityHandle handle);
static EntityHandle handle_from_entity(Entity *e);

static    void remove_entity(Entity* e);
static Entity* add_entity(EntityType type);
static Entity* add_quad(v2 pos, v2 dim, RGBA color);
static Entity* add_texture(u32 texture, v2 pos, v2 dim, RGBA color=WHITE, u32 flags = 0);
static Entity* add_ship(u32 texture, v2 pos, v2 dim, RGBA color=WHITE, u32 flags = 0);
static Entity* add_bullet(u32 texture, v2 pos, v2 dim, f32 deg, RGBA color=WHITE, u32 flags = 0);
static Entity* add_asteroid(u32 texture, v2 pos, v2 dim, f32 deg, RGBA color=WHITE, u32 flags = 0);

static void entities_clear();
static void serialize_data(String8 filename);
static void deserialize_data(String8 filename);
static bool handle_global_events(Event event);
static bool handle_camera_events(Event event);
static bool handle_controller_events(Event event);

static void reset_game(void);
static void reset_ship(void);

static void update_game(Window* window, Memory* memory, Events* events);

#endif

