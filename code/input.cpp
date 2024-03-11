#ifndef INPUT_C
#define INPUT_C

static void
events_init(Events* events){
    events->size = array_count(events->e);
}

static u32
events_count(Events* events){
    u32 result = events->write - events->read;
    return(result);
}

static bool
events_full(Events* events){
    bool result = (events_count(events) == events->size);
    return(result);
}

static bool
events_empty(Events* events){
    bool result = (events->write == events->read);
    return(result);
}

static u32
mask(Events* events, u32 idx){
    u32 result = idx & (events->size - 1);
    return(result);
}

static void
events_add(Events* events, Event event){
    assert(!events_full(events));

    u32 masked_idx = mask(events, events->write++);
    events->e[masked_idx] = event;
}

static Event
events_next(Events* events){
    assert(!events_empty(events));

    u32 masked_idx = mask(events, events->read++);
    Event event = events->e[masked_idx];
    return(event);
}

static void
clear_controller_pressed(Controller* controller){
    controller->up.pressed    = false;
    controller->down.pressed  = false;
    controller->left.pressed  = false;
    controller->right.pressed = false;
    controller->shoot.pressed = false;
    controller->q.pressed     = false;
    controller->e.pressed     = false;
    controller->three.pressed = false;
    controller->four.pressed  = false;

    controller->z.pressed  = false;
    controller->x.pressed  = false;
    controller->c.pressed  = false;
    controller->v.pressed  = false;
    controller->b.pressed  = false;
    controller->n.pressed  = false;
    controller->m.pressed  = false;
    controller->comma.pressed  = false;
    controller->s.pressed  = false;
    controller->d.pressed  = false;
    controller->g.pressed  = false;
    controller->h.pressed  = false;
    controller->j.pressed  = false;
}

#endif
