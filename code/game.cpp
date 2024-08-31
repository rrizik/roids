#ifndef GAME_C
#define GAME_C

static void
init_levels(void){
    Level* level = 0;

    level = state->levels + 0;
    level->asteroid_count_max = 3;
    level->asteroid_spawned = 0;
    level->asteroid_destroyed = 0;

    level = state->levels + 1;
    level->asteroid_count_max = 5;
    level->asteroid_spawned = 0;
    level->asteroid_destroyed = 0;

    level = state->levels + 2;
    level->asteroid_count_max = 7;
    level->asteroid_spawned = 0;
    level->asteroid_destroyed = 0;
}

// todo: Move these to entity once you move PermanentMemory further up in the tool chain
static Entity*
entity_from_handle(EntityHandle handle){
    Entity *result = 0;
    if(handle.index < (s32)array_count(state->entities)){
        Entity *e = state->entities + handle.index;
        if(e->generation == handle.generation){
            result = e;
        }
    }
    return(result);
}

static EntityHandle
handle_from_entity(Entity *e){
    assert(e != 0);
    EntityHandle result = {0};
    if((e >= state->entities) && (e < (state->entities + array_count(state->entities)))){
        result.index = e->index;
        result.generation = e->generation;
    }
    return(result);
}

static void
remove_entity(Entity* e){
    e->type = EntityType_None; // todo: remove this
    clear_flags(&e->flags, EntityFlag_Active);
    state->free_entities[++state->free_entities_at] = e->index;
    state->entities_count--;
    *e = {0};
}

static Entity*
add_entity(EntityType type){
    if(state->free_entities_at < ENTITIES_MAX){
        u32 free_entity_index = state->free_entities[state->free_entities_at--];
        Entity *e = state->entities + free_entity_index;
        e->index = free_entity_index;
        set_flags(&e->flags, EntityFlag_Active);
        state->generation[e->index]++;
        state->entities_count++;
        e->generation = state->generation[e->index]; // CONSIDER: this might not be necessary
        e->type = type;

        return(e);
    }
    return(0);
}

static Entity*
add_quad(v2 pos, v2 dim, RGBA color){
    Entity* e = add_entity(EntityType_Quad);
    if(e){
        e->dir = make_v2(1, 1);
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->dir = make_v2(0, 1);
        e->deg = 90;
    }
    else{
        print("Failed to add entity: Quad\n");
    }
    return(e);
}

static Entity*
add_texture(u32 texture, v2 pos, v2 dim, RGBA color, u32 flags){
    Entity* e = add_entity(EntityType_Texture);
    if(e){
        e->dir = make_v2(1, 1);
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->dir = make_v2(0, 1);
        e->deg = 90;
        e->texture = texture;
    }
    else{
        print("Failed to add entity: Quad\n");
    }
    return(e);
}

static Entity*
add_ship(u32 texture, v2 pos, v2 dim, RGBA color, u32 flags){
    Entity* e = add_entity(EntityType_Ship);
    if(e){
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->deg = -90;
        e->dir = make_v2(0, -1);
        e->accel_dir = make_v2(0, 0);
        e->speed = 200;
        e->velocity = 0;
        e->shoot_t = 1;
        e->texture = texture;
        e->collision_type = CollisionType_Explode;
        e->exploding = false;
        e->explosion_tex = TextureAsset_Explosion1;
        e->explosion_t = 0.0f;
        e->immune_t = 0.0f;
        e->immune = true;
        e->death_type = DeathType_Animate;
        e->health = 1;
        if(flags == 0){
            set_flags(&e->flags, EntityFlag_MoveWithCtrls | EntityFlag_CanCollide | EntityFlag_CanShoot | EntityFlag_Wrapping);
        }
        else{
            set_flags(&e->flags, flags);
        }
    }
    else{
        print("Failed to add entity: Ship\n");
    }
    return(e);
}

static Entity*
add_bullet(u32 texture, v2 pos, v2 dim, f32 deg, RGBA color, u32 flags){
    Entity* e = add_entity(EntityType_Bullet);
    if(e){
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->deg = deg;
        e->dir = dir_from_deg(deg);
        e->speed = 500;
        e->velocity = 1;
        e->damage = 50;
        e->texture = texture;
        e->collision_type = CollisionType_SplinterOnDeath;
        e->death_type = DeathType_Particle;
        e->health = 1;
        if(flags == 0){
            set_flags(&e->flags, EntityFlag_MoveWithPhys | EntityFlag_CanCollide | EntityFlag_IsProjectile);
        }
        else{
            set_flags(&e->flags, flags);
        }
    }
    else{
        print("Failed to add entity: Bullet\n");
    }
    return(e);
}

static Entity*
add_bullet_particle(u32 texture, v2 pos, v2 dim, f32 deg, RGBA color, u32 flags){
    Entity* e = add_entity(EntityType_Particle);
    if(e){
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->deg = deg;
        e->dir = dir_from_deg(deg);
        e->speed = 500;
        e->velocity = 1;
        e->damage = 50;
        e->texture = texture;
        e->collision_type = CollisionType_SplinterOnDeath;
        e->death_type = DeathType_Particle;
        e->particle_type = ParticleType_Bullet;
        e->health = 1;
        if(flags == 0){
            set_flags(&e->flags, EntityFlag_MoveWithPhys | EntityFlag_Particle);
        }
        else{
            set_flags(&e->flags, flags);
        }
    }
    else{
        print("Failed to add entity: Bullet Particle\n");
    }
    return(e);
}

static Entity*
add_asteroid(u32 texture, v2 pos, v2 dim, f32 deg, RGBA color, u32 flags){
    Entity* e = add_entity(EntityType_Asteroid);
    if(e){
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->deg = deg;
        e->dir = dir_from_deg(deg);
        e->speed = 100;
        e->rot_speed = (f32)random_range_u32(150) + 50;
        e->velocity = 1;
        e->health = (s32)dim.w;
        e->texture = texture;
        e->collision_type = CollisionType_HealthOrSplinter;
        e->death_type = DeathType_Crumble;
        if(flags == 0){
            set_flags(&e->flags, EntityFlag_MoveWithPhys | EntityFlag_CanCollide | EntityFlag_Wrapping);
        }
        else{
            set_flags(&e->flags, flags);
        }
    }
    else{
        print("Failed to add entity: Asteroid\n");
    }
    return(e);
}

static void
entities_clear(void){
    state->free_entities_at = ENTITIES_MAX - 1;
    for(u32 i = state->free_entities_at; i <= state->free_entities_at; --i){
        Entity* e = state->entities + i;
        e->type = EntityType_None;
        clear_flags(&e->flags, EntityFlag_Active);
        state->free_entities[i] = state->free_entities_at - i;
        state->generation[i] = 0;
    }
    state->entities_count = 0;
}

static void
serialize_data(String8 filename){
    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, saves_path, filename);

    File file = os_file_open(full_path, GENERIC_WRITE, CREATE_NEW);
    if(file.handle != INVALID_HANDLE_VALUE){
        os_file_write(file, state->entities, sizeof(Entity) * ENTITIES_MAX);
    }
    else{
        // log error
        print("Save file \"%s\" already exists: Could not serialize data\n", filename.str);
    }

    os_file_close(file);
    end_scratch(scratch);
}

static void
deserialize_data(String8 filename){
    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, saves_path, filename);

    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    if(!file.size){
        //todo: log error
        print("Error: failed to open file <%s>\n", full_path.str);
        os_file_close(file);
        end_scratch(scratch);
        return;
    }

    String8 data = os_file_read(&state->arena, file);
    entities_clear();

    u32 offset = 0;
    while(offset < data.size){
        Entity* e = (Entity*)(data.str + offset);
        switch(e->type){
            case EntityType_Ship:{
                Entity* ship = add_entity(EntityType_Ship);
                *ship = *e;
                ship->texture = TextureAsset_Ship;

                state->ship = ship;
                state->ship_loaded = true;
            } break;
            case EntityType_Bullet:{
                Entity* bullet = add_entity(EntityType_Bullet);
                *bullet = *e;
                bullet->texture = TextureAsset_Bullet;
            } break;
            case EntityType_Asteroid:{
                Entity* ast = add_entity(EntityType_Asteroid);
                *ast = *e;
                ast->texture = TextureAsset_Asteroid;
            } break;
        }
        offset += sizeof(Entity);
    }
    os_file_close(file);
    end_scratch(scratch);
}

static bool
handle_global_events(Event event){
    if(event.type == QUIT){
        should_quit = true;
        return(true);
    }
    // todo: why is this here
    if(event.type == MOUSE){
        v2 direction = direction_v2(window.dim, controller.mouse.pos);

        f32 rad = rad_from_dir(direction);
        f32 deg = deg_from_dir(direction);
    }
    if(event.type == KEYBOARD){
        if(event.key_pressed){
            if(event.keycode == KeyCode_TILDE && !event.repeat){
                if(event.shift_pressed){
                    if(console.state == OPEN_BIG){
                        console_set_state(CLOSED);
                    }
                    else{
                        console_set_state(OPEN_BIG);
                    }
                }
                else{
                    if(console.state == OPEN || console.state == OPEN_BIG){
                        console_set_state(CLOSED);
                    }
                    else{
                        console_set_state(OPEN);
                    }

                }
                return(true);
            }
        }
        else{
        }
    }
    return(false);
}

static bool
handle_camera_events(Event event){
    if(event.type == MOUSE){
        controller.mouse.x = event.mouse_x;
        controller.mouse.y = event.mouse_y;
        controller.mouse.dx = event.mouse_dx;
        controller.mouse.dy = event.mouse_dy;
    }
    if(event.type == KEYBOARD){
        if(event.key_pressed){
            if(event.keycode == KeyCode_ONE){
                if(state->game_mode != GameMode_FirstPerson){
                    state->game_mode = GameMode_FirstPerson;

                    controller.mouse.dx = 0;
                    controller.mouse.dy = 0;

                    POINT center = {((s32)window.width/2), ((s32)window.height/2)};
                    ClientToScreen(window.handle, &center);
                    show_cursor(false);
                }
            }
            if(event.keycode == KeyCode_TWO){
                if(state->game_mode != GameMode_Editor){
                    state->game_mode = GameMode_Editor;
                    show_cursor(true);
                }
            }
            if(event.keycode == KeyCode_THREE){
                if(state->game_mode != GameMode_Game){
                    state->game_mode = GameMode_Game;
                    init_camera();
                    show_cursor(false);
                }
            }
        }
    }
    return(false);
}

static bool
handle_controller_events(Event event){
    controller.mouse.dx = 0;
    controller.mouse.dy = 0;
    controller.mouse.wheel_dir = 0;
    if(event.type == MOUSE){
        controller.mouse.x = event.mouse_x;
        controller.mouse.y = event.mouse_y;
        controller.mouse.dx = event.mouse_dx;
        controller.mouse.dy = event.mouse_dy;
        controller.mouse.wheel_dir = event.mouse_wheel_dir;
    }
    if(event.type == KEYBOARD){
        if(event.key_pressed){
            if(!event.repeat){
                controller.button[event.keycode].pressed = true;
            }
            controller.button[event.keycode].held = true;
        }
        else{
            controller.button[event.keycode].held = false;
        }
    }
    return(false);
}

static bool
handle_game_events(Event event){
    if(event.type == KEYBOARD){
        if(event.key_pressed){
            if(event.keycode == KeyCode_ESCAPE){
                if(state->game_mode == GameMode_Game){
                    if(!game_won() && state->lives){
                        pause = !pause;
                        return(true);
                    }
                }
                else{
                    should_quit = true;
                }
            }
        }
    }
    return(false);
}

static void
reset_game(void){
    state->lives = MAX_LIVES;
    state->score = 0;
    state->level_index = 0;
    init_levels();
    state->current_level = &state->levels[0];

    reset_ship();
}

static void
reset_ship(void){
    state->ship->dir = make_v2(0, -1);
    state->ship->pos = make_v2(0, 0);
    state->ship->deg = -90;
    state->ship->dir = dir_from_deg(state->ship->deg);
    state->ship->accel_dir = make_v2(0, 0);
    state->ship->velocity = 0;
    state->ship->exploding = false;
    state->ship->explosion_tex = TextureAsset_Explosion1;
    state->ship->texture = TextureAsset_Ship;
    state->ship->immune = true;
    state->ship->immune_t = 0.0f;
    state->ship->health = 1;
    set_flags(&state->ship->flags, EntityFlag_Active | EntityFlag_MoveWithCtrls | EntityFlag_CanCollide | EntityFlag_CanShoot | EntityFlag_Wrapping);
}

static bool
game_won(void){
    if(state->level_index < MAX_LEVELS){
        return(false);
    }

    Level level = state->levels[state->level_index];
    if(level.asteroid_destroyed < level.asteroid_spawned){
        return(false);
    }

    return(true);
}

static void update_game(void){


    if(state->game_mode == GameMode_Game && !game_won()){
        Level* level   = state->current_level;
        Entity* ship   = state->ship;
        Rect ship_rect = rect_from_entity(ship);

        if(ship->immune_t < 2){
            ship->immune_t += (f32)clock.dt;
        }
        else{
            ship->immune = false;
        }

        // ship behavior
        if(has_flags(ship->flags, EntityFlag_Active)){
            if(has_flags(ship->flags, EntityFlag_MoveWithCtrls)){
                ship->pos.x += (ship->accel_dir.x * ship->velocity * ship->speed) * (f32)clock.dt;
                ship->pos.y += (ship->accel_dir.y * ship->velocity * ship->speed) * (f32)clock.dt;

                if(controller.button[KeyCode_SPACEBAR].held){
                    ship->shoot_t += (f32)clock.dt;
                    if(ship->shoot_t >= 0.1f){
                        ship->shoot_t = 0.0;
                        v2 pos = make_v2(ship->pos.x + (50 * ship->dir.x), ship->pos.y + (50 * ship->dir.y));
                        Entity* child_e = add_bullet(TextureAsset_Bullet, pos, make_v2(40, 8), ship->deg);
                        child_e->origin = ship;

                        // play rail audio
                        u32 random_rail = random_range_u32(5) + 6; // todo: hard coded for now, 5 rails 6 offset
                        wasapi_play(&ts->assets.waves[random_rail], 0.1f, false);
                    }
                }
                else{
                    ship->shoot_t = 1;
                }

                // rotate left right
                if(controller.button[KeyCode_RIGHT].held || controller.button[KeyCode_D].held){
                    f32 d = deg_from_dir(ship->dir);
                    d += 200 * (f32)clock.dt;
                    ship->deg = d;
                    ship->dir = dir_from_deg(d);
                }
                if(controller.button[KeyCode_LEFT].held || controller.button[KeyCode_A].held){
                    f32 d = deg_from_dir(ship->dir);
                    d -= 200 * (f32)clock.dt;
                    ship->deg = d;
                    ship->dir = dir_from_deg(d);
                }

                // increase velocity
                if(controller.button[KeyCode_UP].held || controller.button[KeyCode_W].held){
                    ship->accelerating = true;
                    ship->velocity += (f32)clock.dt;
                    if(ship->velocity > 1){
                        ship->velocity = 1;
                    }
                    ship->accel_dir.x += ship->dir.x/100;
                    ship->accel_dir.y += ship->dir.y/100;
                    clamp_f32(-1, 1, &ship->accel_dir.x);
                    clamp_f32(-1, 1, &ship->accel_dir.y);
                }
                else{
                    ship->accelerating = false;
                    ship->velocity -= (f32)clock.dt/4;
                    if(ship->velocity < 0){
                        ship->velocity = 0;
                    }
                }
            }
        }

        // advance level
        if(level->asteroid_spawned == level->asteroid_count_max &&
           level->asteroid_spawned == level->asteroid_destroyed){
            state->level_index++;
            if(state->level_index < MAX_LEVELS){
                state->current_level = &state->levels[state->level_index];
            }
        }

        // spawn asteroids
        state->spawn_t += clock.dt;
        if(state->spawn_t >= 0.5f){
            state->spawn_t = 0.0;

            v2 dim;
            dim.x = random_range_f32(150) + 50;
            dim.y = dim.x;
            u32 side = random_range_u32(3);

            v2 pos = {0, 0};
            f32 deg = 0;
            if(side == 0){
                pos.x = state->screen_left -200;
                pos.y = random_range_f32(state->screen_bottom - 1) - state->screen_bottom;
                deg = random_range_f32(180) - 90.0f;
            }
            if(side == 1){
                pos.x = state->screen_right + 200.0f;
                pos.y = random_range_f32(state->screen_bottom - 1) - state->screen_bottom;

                s32 sign = (s32)random_range_u32(1) - 1;
                deg = (random_range_f32(90) + 90.0f) * (f32)sign;
            }
            if(side == 2){
                pos.x = random_range_f32(state->screen_right - 1) - state->screen_right;
                pos.y = state->screen_top -200;

                deg = random_range_f32(180);
            }
            if(side == 3){
                pos.x = random_range_f32(state->screen_right - 1) - state->screen_right;
                pos.y = state->screen_bottom + 200.0f;

                deg = random_range_f32(180) - 180;
            }
            if(state->current_level->asteroid_spawned < state->current_level->asteroid_count_max){
                Entity* asteroid = add_asteroid(TextureAsset_Asteroid, pos, dim, deg);
                state->current_level->asteroid_spawned++;
            }
        }

        // resolve entity motion
        for(s32 i = 0; i < array_count(state->entities); ++i){
            begin_timed_scope("entity_motion");
            Entity *e = state->entities + i;
            if(!has_flags(e->flags, EntityFlag_Active)){
                continue;
            }

            if(has_flags(e->flags, EntityFlag_MoveWithPhys)){
                e->pos.x += (e->dir.x * e->velocity * e->speed) * (f32)clock.dt;
                e->pos.y += (e->dir.y * e->velocity * e->speed) * (f32)clock.dt;
            }

            if(has_flags(e->flags, EntityFlag_Wrapping)){
                if(e->pos.x - e->dim.w/2 > state->screen_right){
                    e->pos.x = state->screen_left - e->dim.w/2;
                }
                if(e->pos.y - e->dim.h/2 > state->screen_bottom){
                    e->pos.y = state->screen_top - e->dim.h/2;
                }
                if(e->pos.x + e->dim.w/2 < state->screen_left){
                    e->pos.x = state->screen_right + e->dim.w/2;
                }
                if(e->pos.y + e->dim.h/2 < state->screen_top){
                    e->pos.y = state->screen_bottom + e->dim.h/2;
                }
            }
            else{
                if((e->pos.x + e->dim.w/2 < -window.width/2)   ||
                   (e->pos.y + e->dim.h/2 < -window.height/2)  ||
                   (e->pos.x - e->dim.w/2 >  window.width/2)   ||
                   (e->pos.y - e->dim.h/2 >  window.height/2)){
                   remove_entity(e);
                }
            }
        }

        // resolve death
        for(s32 i = 0; i < array_count(state->entities); ++i){
            begin_timed_scope("entity_motion");
            Entity *e = state->entities + i;
            if(!has_flags(e->flags, EntityFlag_Active)){
                continue;
            }

            //if(e->dead){
            //}
            if(e->health <= 0){
                switch(e->death_type){
                    case DeathType_Crumble:{
                        if(e->dim.w > 100){
                            e->dim.w -= 50;
                            e->dim.h -= 50;
                            for(s32 splint_i=0; splint_i < 3; ++splint_i){
                                e->deg = random_range_f32(360);
                                add_asteroid(TextureAsset_Asteroid, e->pos, e->dim, e->deg);
                                state->current_level->asteroid_spawned++;
                                state->current_level->asteroid_count_max++;
                            }
                        }

                        state->score += (u32)e->dim.w;
                        state->current_level->asteroid_destroyed++;
                        remove_entity(e);
                    } break;
                    case DeathType_Particle:{
                        s32 p_count = (s32)random_range_u32(5) + 5;
                        for(s32 p_idx = 0; p_idx < p_count; p_idx++){
                            f32 deg = random_range_f32(360);
                            v2 dim = make_v2(10, 2);
                            Entity* e_p = add_bullet_particle(TextureAsset_Bullet, e->pos, dim, deg);
                            e_p->parent = state->ship;
                            e_p->particle_t = 0.175f;
                        }
                        remove_entity(e);
                    } break;
                    case DeathType_Animate:{
                    } break;
                }
            }
        }

        // resolve particles
        for(s32 i = 0; i < array_count(state->entities); ++i){
            Entity *e = state->entities + i;
            if(!has_flags(e->flags, EntityFlag_Active)){
                continue;
            }

            if(has_flags(e->flags, EntityFlag_Particle)){
                switch(e->particle_type){
                    case ParticleType_Bullet:{
                        if(e->dim.w > 0){
                            e->dim.w -= (f32)clock.dt * 40;
                        }
                        else{
                            remove_entity(e);
                        }
                    }
                }
            }
        }
            //Rect e_rect = rect_from_entity(e);
            //if(!has_flags(e, EntityFlag_CanCollide)){
            //    for(s32 j = 0; j < array_count(state->entities); ++j){
            //        Entity *e_inner = state->entities + j;
            //        if(e == e_inner->parent){
            //            continue;
            //        }

            //        Rect e_inner_rect = rect_from_entity(e_inner);
            //        if(rect_collides_rect(e_inner_rect, e_rect)){

            //        }
            //    }
            //}

        // type loop
        for(s32 i = 0; i < array_count(state->entities); ++i){
            begin_timed_scope("type_loop");
            Entity *e = state->entities + i;
            Rect e_rect = rect_from_entity(e);

            switch(e->type){
                case EntityType_Ship:{

                    if(has_flags(state->ship->flags, EntityFlag_Active)){
                        if(e->exploding){
                            e->accelerating = false;
                            e->texture = e->explosion_tex;
                            e->explosion_t += (f32)clock.dt;
                            if(e->explosion_t >= 0.030f){
                                if(e->texture != TextureAsset_Explosion6){
                                    e->explosion_tex++;
                                }
                                else{
                                    if(state->lives){
                                        reset_ship();
                                    } else{
                                        clear_flags(&state->ship->flags, EntityFlag_Active);
                                    }
                                }
                                e->explosion_t = 0.0f;
                            }
                        }
                        else{

#if 1
                            if(!e->immune){
                                for(s32 idx = 0; idx < array_count(state->entities); ++idx){
                                    Entity *collide_e = state->entities + idx;
                                    if(collide_e->type == EntityType_Asteroid){
                                        Rect collide_e_rect = rect_from_entity(collide_e);
                                        if(rect_collides_rect(e_rect, collide_e_rect)){
                                            state->lives -= 1;
                                            if(state->lives == 0){
                                                e->dead = true;
                                            }
                                            e->health = 0;
                                            e->exploding = true;
                                            clear_flags(&e->flags, EntityFlag_MoveWithCtrls);
                                            state->current_level->asteroid_destroyed++;
                                            remove_entity(collide_e);
                                            break;
                                        }
                                    }
                                }
                            }
#endif
                        }
                    }
                } break;
                case EntityType_Bullet:{
                    Rect rect_e = rect_from_entity(e);
                    if(!has_flags(e->flags, EntityFlag_CanCollide)){
                        break;
                    }
                    for(s32 idx = 0; idx < array_count(state->entities); ++idx){
                        Entity *collide_e = state->entities + idx;

                        if(collide_e == e->origin){
                            continue;
                        }
                        if(collide_e->type == EntityType_Asteroid){
                            Rect collide_e_rect = rect_from_entity(collide_e);

                            if(rect_collides_rect(collide_e_rect, rect_e)){

                                collide_e->health -= e->damage;
                                collide_e->color.r += 0.2f;
                                collide_e->color.g -= 0.4f;
                                collide_e->color.b -= 0.4f;

                                e->health = 0;
                                e->dead = true;
                                break;
                            }
                        }
                    }
                } break;
                case EntityType_Asteroid:{
                    e->deg += e->rot_speed * (f32)clock.dt;
                } break;
            }
        }
    }
}

#endif

