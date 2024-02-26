#ifndef ENTITY_H
#define ENTITY_H

typedef enum {EntityFlag_Movable} EntityFlags;
typedef enum {EntityType_None, EntityType_Quad, EntityType_Texture, EntityType_Text, EntityType_Object, EntityType_Pixel, EntityType_Line, EntityType_Ray, EntityType_Segment, EntityType_Triangle, EntityType_Rect, EntityType_Box, EntityType_Circle, EntityType_Bitmap, EntityType_Glyph, EntityType_Bases, EntityType_Ship, EntityType_Bullet, EntityType_Cube, EntityType_Player} EntityType;

typedef struct Entity{
    u32 index;
    u32 generation;

    EntityType type;
    ID3D11ShaderResourceView** texture;

    // TODO: Remove this
    f32 x;
    f32 y;
    String8 text;
    Font font;

    // TODO: Review this
    u32 flags;
    Rect rect;
    v2 origin;
    v2 x_axis;
    v2 y_axis;
    RGBA color;
    s32 border_size;
    RGBA border_color;
    s32 z;
    f32 start_position;
    v3 pos;
    v3 angle;
    v3 scale;
    v2 p0;
    v2 p1;
    v2 p2;
    v2 p3;

    String8 name;
    v2 direction;
    v2 target_direction;
    f32 rotation_percent;
    f32 rad;

    f32 speed;

    f32 velocity;

    bool draw;
    bool fill;
} Entity;

static bool has_flags(u32 flags, u32 flags_set);
static void set_flags(u32* flags, u32 flags_set);
static void clear_flags(u32* flags, u32 flags_set);

typedef struct EntityHandle{
    u32 index;
    u32 generation;
} EntityHandle;

static EntityHandle zero_entity_handle();

#endif
