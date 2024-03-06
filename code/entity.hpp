#ifndef ENTITY_H
#define ENTITY_H

typedef enum {EntityFlag_Movable} EntityFlags;
typedef enum {EntityType_None, EntityType_Quad, EntityType_Texture, EntityType_Text, EntityType_Line, EntityType_Ship, EntityType_Bullet, EntityType_Asteroid} EntityType;

typedef struct Entity{
    EntityType type;
    u32 flags;
    u32 index;
    u32 generation;

    v2 pos;
    v2 dim;
    v2 dir;
    f32 deg;
    v2 scale;
    v2 origin;
    RGBA color;

    f32 speed;
    f32 velocity;

    ID3D11ShaderResourceView** texture;
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


#endif
