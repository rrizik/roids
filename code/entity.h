#ifndef ENTITY_H
#define ENTITY_H

typedef enum {EntityFlag_Movable} EntityFlags;
typedef enum {EntityType_None, EntityType_Object, EntityType_Pixel, EntityType_Line, EntityType_Ray, EntityType_Segment, EntityType_Triangle, EntityType_Rect, EntityType_Quad, EntityType_Box, EntityType_Circle, EntityType_Bitmap, EntityType_Glyph, EntityType_Basis} EntityType;

typedef struct Basis{
    v2 origin;
    v2 x_axis;
    v2 y_axis;
} Basis;

typedef struct Entity{
    u32 index;
    u32 generation;

    EntityType type;
    //ConsoleState console_state;
    u32 flags;
    Rect rect;
    Basis basis;
    RGBA color;
    s32 border_size;
    RGBA border_color;
    s32 z;
    f32 start_position;


    v2 direction;
    f32 rad;

    f32 speed;

    v2 p0 = p0;
    v2 p1 = p1;
    v2 p2 = p2;
    v2 p3 = p3;

    bool draw;
    bool fill;

    Bitmap image;
    Glyph glyph;
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
