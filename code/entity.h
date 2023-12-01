#ifndef ENTITY_H
#define ENTITY_H

typedef enum {EntityFlag_Movable} EntityFlags;
typedef enum {EntityType_None, EntityType_Object, EntityType_Pixel, EntityType_Line, EntityType_Ray, EntityType_Segment, EntityType_Triangle, EntityType_Rect, EntityType_Quad, EntityType_Box, EntityType_Circle, EntityType_Bitmap, EntityType_Glyph, EntityType_Bases, EntityType_Ship, EntityType_Bullet, EntityType_Cube, EntityType_Player} EntityType;

typedef struct Entity{
    u32 index;
    u32 generation;

    EntityType type;

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


    String8 name;
    v2 direction;
    f32 rad;

    f32 speed;

    v2 p0 = p0;
    v2 p1 = p1;
    v2 p2 = p2;
    v2 p3 = p3;

    f32 velocity;

    bool draw;
    bool fill;

    // QUESTION: Should this be a pointer
    Bitmap* texture;
    bool render;
} Entity;

static bool
has_flags(u32 flags, u32 flags_set){
    return((flags & flags_set) == flags_set);
}

static void
set_flags(u32* flags, u32 flags_set){
    *flags |= flags_set;
}

static void
clear_flags(u32* flags, u32 flags_set){
    *flags &= ~flags_set;
}

typedef struct EntityHandle{
    u32 index;
    u32 generation;
} EntityHandle;

static EntityHandle
zero_entity_handle(){
    EntityHandle result = {0};
    return(result);
}

#endif
