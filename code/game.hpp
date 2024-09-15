#ifndef GAME_H
#define GAME_H

typedef struct Level{
    s32 asteroid_spawned;
    s32 asteroid_destroyed;
    s32 asteroid_count_max;
} Level;


typedef enum SceneState{
    SceneState_None,
    SceneState_Menu,
    SceneState_Game,
} SceneState;

typedef enum GameState{
    GameState_None,
    GameState_Running,
    GameState_Paused,
    GameState_Finished,
} GameMode;

typedef enum GameResult{
    GameResult_None,
    GameResult_Won,
    GameResult_Lost,
} GameResult;


static v2 world_from_screen_space(v2 point);

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
static Entity* add_bullet_particle(u32 texture, v2 pos, v2 dim, f32 deg, RGBA color=WHITE, u32 flags = 0);
static Entity* add_asteroid(u32 texture, v2 pos, v2 dim, f32 deg, RGBA color=WHITE, u32 flags = 0);

static void entities_clear(void);
static void serialize_data(String8 filename);
static void deserialize_data(String8 filename);
static bool handle_global_events(Event event);
static bool handle_camera_events(Event event);
static bool handle_controller_events(Event event);
static bool handle_game_events(Event event);


static void update_game(void);
static void init_levels(void);
static void ship_reset(void);
static bool game_won(void);
static bool game_lost(void);
static void game_reset(void);

#endif

