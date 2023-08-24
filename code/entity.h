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
    v3 scale;
    v2 origin;
    v2 x_axis;
    v2 y_axis;
    RGBA color;
    s32 border_size;
    RGBA border_color;
    s32 z;
    f32 start_position;
    v3 pos;


    String8 name;
    v2 direction;
    f32 rad;
    f32 angle;

    f32 speed;

    v2 p0 = p0;
    v2 p1 = p1;
    v2 p2 = p2;
    v2 p3 = p3;

    f32 velocity;

    bool draw;
    bool fill;

    Bitmap texture;
    Bitmap glyph;
    bool render;
} Entity;

static bool
has_flags(Entity *e, u32 flags){
    return(e->flags & flags);
}

static void
set_flags(Entity *e, u32 flags){
    e->flags |= flags;
}

static void
clear_flags(Entity *e, u32 flags){
    e->flags &= ~flags;
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
