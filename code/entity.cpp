#ifndef ENTITY_C
#define ENTITY_C

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

static EntityHandle
zero_entity_handle(){
    EntityHandle result = {0};
    return(result);
}

static Rect
rect_from_entity(Entity* e){
    Rect result = make_rect(make_v2(e->pos.x - e->dim.w/2, e->pos.y - e->dim.h/2),
                            make_v2(e->pos.x + e->dim.x/2, e->pos.y + e->dim.h/2));
    return(result);
}

#endif
