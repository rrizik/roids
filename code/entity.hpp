#ifndef ENTITY_H
#define ENTITY_H

typedef enum EntityFlag {
    EntityFlag_Active =        (1 << 0),
    EntityFlag_MoveWithCtrls = (1 << 1),
    EntityFlag_CanCollide =    (1 << 2),
    EntityFlag_CanShoot =      (1 << 3),
    EntityFlag_MoveWithPhys =  (1 << 4),
    EntityFlag_IsProjectile =  (1 << 5),
} EntityFlag;

typedef enum EntityType {EntityType_None, EntityType_Quad, EntityType_Texture, EntityType_Text, EntityType_Line, EntityType_Ship, EntityType_Bullet, EntityType_Asteroid} EntityType;

typedef struct Entity{
    Entity* next;
    Entity* prev;

    EntityType type;
    u32 flags;
    u32 index;
    u32 generation;

    v2 pos;
    v2 dim;
    v2 dir;
    v2 accel_dir;
    f32 deg;
    v2 scale;
    RGBA color;

    f32 speed;
    f32 rot_speed;
    f32 velocity;

    s32 health;
    s32 damage;
    bool in_play;

    u32 texture;
} Entity;

static bool has_flags(u32 flags, u32 flags_set);
static void set_flags(u32* flags, u32 flags_set);
static void clear_flags(u32* flags, u32 flags_set);

typedef struct EntityHandle{
    u32 index;
    u32 generation;
} EntityHandle;

static EntityHandle zero_entity_handle();
//static Entity* entity_from_handle(PermanentMemory* pm, EntityHandle handle);
//static EntityHandle handle_from_entity(PermanentMemory* pm, Entity *e);
static Rect rect_from_entity(Entity* e);


#endif
