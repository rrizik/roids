#ifndef GAME_H
#define GAME_H

typedef enum GameMode{
    GameMode_FirstPerson,
    GameMode_Editor,
    GameMode_Game,
} GameMode;

static void load_assets(Arena* arena, Assets* assets);

static Bitmap* get_bitmap(Assets* assets, AssetID id);

// todo: PermanentMemory is global, stop passing it around
static Entity* entity_from_handle(PermanentMemory* pm, EntityHandle handle);
static EntityHandle handle_from_entity(PermanentMemory* pm, Entity *e);

static    void remove_entity(PermanentMemory* pm, Entity* e);
static Entity* add_entity(PermanentMemory *pm, EntityType type);
static Entity* add_quad(PermanentMemory* pm, v2 pos, v2 dim, RGBA color);
static Entity* add_texture(PermanentMemory* pm, ID3D11ShaderResourceView** texture, v2 pos, v2 dim, RGBA color=WHITE);
static Entity* add_ship(PermanentMemory* pm, ID3D11ShaderResourceView** texture, v2 pos, v2 dim, RGBA color=WHITE);
static Entity* add_bullet(PermanentMemory* pm, ID3D11ShaderResourceView** texture, v2 pos, v2 dim, f32 deg, RGBA color=WHITE);
static Entity* add_asteroid(PermanentMemory* pm, ID3D11ShaderResourceView** texture, v2 pos, v2 dim, f32 deg, RGBA color=WHITE);

static void entities_clear(PermanentMemory* pm);
static void serialize_data(PermanentMemory* pm, String8 filename);
static void deserialize_data(PermanentMemory* pm, String8 filename);
static bool handle_global_events(Event event);
static bool handle_camera_events(Event event);
static bool handle_controller_events(Event event);

static void update_game(Window* window, Memory* memory, Events* events);

#endif

