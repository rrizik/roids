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

#endif
