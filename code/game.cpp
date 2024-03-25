#ifndef GAME_C
#define GAME_C

static void
load_assets(Arena* arena, Assets* assets){
    assets->bitmap[BitmapAsset_Ship] =   load_bitmap(arena, str8_literal("sprites/ship2.bmp"));
    assets->bitmap[BitmapAsset_Circle] = load_bitmap(arena, str8_literal("sprites/circle.bmp"));
    assets->bitmap[BitmapAsset_Asteroid] = load_bitmap(arena, str8_literal("sprites/asteroid.bmp"));

    assets->waves[WaveAsset_track1] = load_wave(arena, str8_literal("sounds/track1.wav"));
    assets->waves[WaveAsset_track2] = load_wave(arena, str8_literal("sounds/track2.wav"));
    assets->waves[WaveAsset_track3] = load_wave(arena, str8_literal("sounds/track3.wav"));
    assets->waves[WaveAsset_track4] = load_wave(arena, str8_literal("sounds/track4.wav"));
    assets->waves[WaveAsset_track5] = load_wave(arena, str8_literal("sounds/track5.wav"));
    assets->waves[WaveAsset_bullet] = load_wave(arena, str8_literal("sounds/bullet.wav"));

    assets->fonts[FontAsset_Arial] = load_font_ttf(arena, str8_literal("fonts/arial.ttf"), 36);
    assets->fonts[FontAsset_Golos] = load_font_ttf(arena, str8_literal("fonts/GolosText-Regular.ttf"), 24);
}

// todo: Move these to entity once you move PermanentMemory further up in the tool chain
static Entity*
entity_from_handle(PermanentMemory* pm, EntityHandle handle){
    Entity *result = 0;
    if(handle.index < (s32)array_count(pm->entities)){
        Entity *e = pm->entities + handle.index;
        if(e->generation == handle.generation){
            result = e;
        }
    }
    return(result);
}

static EntityHandle
handle_from_entity(PermanentMemory* pm, Entity *e){
    assert(e != 0);
    EntityHandle result = {0};
    if((e >= pm->entities) && (e < (pm->entities + array_count(pm->entities)))){
        result.index = e->index;
        result.generation = e->generation;
    }
    return(result);
}

static void
remove_entity(PermanentMemory* pm, Entity* e){
    pm->free_entities[++pm->free_entities_at] = e->index;
    pm->entities_count--;
    *e = {0};
}

static Entity*
add_entity(PermanentMemory *pm, EntityType type){
    if(pm->free_entities_at < ENTITIES_MAX){
        u32 free_entity_index = pm->free_entities[pm->free_entities_at--];
        Entity *e = pm->entities + free_entity_index;
        e->index = free_entity_index;
        pm->generation[e->index]++;
        pm->entities_count++;
        e->generation = pm->generation[e->index]; // CONSIDER: this might not be necessary
        e->type = type;

        return(e);
    }
    return(0);
}

static Entity*
add_quad(PermanentMemory* pm, v2 pos, v2 dim, RGBA color){
    Entity* e = add_entity(pm, EntityType_Quad);
    if(e){
        e->dir = make_v2(1, 1);
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->dir = make_v2(0, 1);
        e->deg = 90;
        //e->origin = make_v2((pos.x + (pos.x + dim.w))/2, (pos.y + (pos.y + dim.h))/2);
    }
    else{
        print("Failed to add entity: Quad\n");
    }
    return(e);
}

static Entity*
add_texture(PermanentMemory* pm, ID3D11ShaderResourceView** texture, v2 pos, v2 dim, RGBA color){
    Entity* e = add_entity(pm, EntityType_Texture);
    if(e){
        e->dir = make_v2(1, 1);
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->dir = make_v2(0, 1);
        e->deg = 90;
        //e->origin = make_v2((pos.x + (pos.x + dim.w))/2, (pos.y + (pos.y + dim.h))/2);
        e->texture = texture;
    }
    else{
        print("Failed to add entity: Quad\n");
    }
    return(e);
}

static Entity*
add_ship(PermanentMemory* pm, ID3D11ShaderResourceView** texture, v2 pos, v2 dim, RGBA color){
    Entity* e = add_entity(pm, EntityType_Ship);
    if(e){
        e->dir = make_v2(1, 1);
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->dir = make_v2(0, -1);
        e->deg = -90;
        e->speed = 400;
        e->velocity = 0;
        e->texture = texture;
    }
    else{
        print("Failed to add entity: Ship\n");
    }
    return(e);
}

static Entity*
add_bullet(PermanentMemory* pm, ID3D11ShaderResourceView** texture, v2 pos, v2 dim, f32 deg, RGBA color){
    Entity* e = add_entity(pm, EntityType_Bullet);
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
    }
    else{
        print("Failed to add entity: Bullet\n");
    }
    return(e);
}

static Entity*
add_asteroid(PermanentMemory* pm, ID3D11ShaderResourceView** texture, v2 pos, v2 dim, f32 deg, RGBA color){
    Entity* e = add_entity(pm, EntityType_Asteroid);
    if(e){
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->deg = deg;
        e->dir = dir_from_deg(deg);
        e->speed = 200;
        e->rot_speed = (f32)random_range_u32(150) + 50;
        e->velocity = 1;
        e->health = (s32)dim.w;
        e->texture = texture;
    }
    else{
        print("Failed to add entity: Asteroid\n");
    }
    return(e);
}

static void
entities_clear(PermanentMemory* pm){
    pm->free_entities_at = ENTITIES_MAX - 1;
    for(u32 i = pm->free_entities_at; i <= pm->free_entities_at; --i){
        Entity* e = pm->entities + i;
        e->type = EntityType_None;
        pm->free_entities[i] = pm->free_entities_at - i;
        pm->generation[i] = 0;
    }
    pm->entities_count = 0;
}

static void
serialize_data(PermanentMemory* pm, String8 filename){
    ScratchArena scratch = begin_scratch(0);
    String8 full_path = str8_path_append(scratch.arena, saves_path, filename);

    File file = os_file_open(full_path, GENERIC_WRITE, CREATE_NEW);
    if(file.handle != INVALID_HANDLE_VALUE){
        os_file_write(file, pm->entities, sizeof(Entity) * ENTITIES_MAX);
    }
    else{
        // log error
        print("Save file \"%s\" already exists: Could not serialize data\n", filename.str);
    }

    os_file_close(file);
    end_scratch(scratch);
}

static void
deserialize_data(PermanentMemory* pm, String8 filename){
    ScratchArena scratch = begin_scratch(0);
    String8 full_path = str8_path_append(scratch.arena, saves_path, filename);

    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    if(!file.size){
        //todo: log error
        print("Error: failed to open file <%s>\n", full_path.str);
        os_file_close(file);
        end_scratch(scratch);
        return;
    }

    String8 data = os_file_read(&pm->arena, file);
    entities_clear(pm);

    u32 offset = 0;
    while(offset < data.size){
        Entity* e = (Entity*)(data.str + offset);
        switch(e->type){
            case EntityType_Ship:{
                Entity* ship = add_entity(pm, EntityType_Ship);
                *ship = *e;
                ship->texture = &ship_shader_resource;

                pm->ship = ship;
                pm->ship_loaded = true;
            } break;
            case EntityType_Bullet:{
                Entity* bullet = add_entity(pm, EntityType_Bullet);
                *bullet = *e;
                bullet->texture = &bullet_shader_resource;
            } break;
            case EntityType_Asteroid:{
                Entity* ast = add_entity(pm, EntityType_Asteroid);
                *ast = *e;
                ast->texture = &asteroid_shader_resource;
            } break;
        }
        offset += sizeof(Entity);
    }
    os_file_close(file);
    end_scratch(scratch);
}

static v2 dir_normalized = make_v2(0, 0);
static bool
handle_global_events(Event event){
    if(event.type == QUIT){
        should_quit = true;
        return(true);
    }
    if(event.type == MOUSE){
        v2 p0 = make_v2(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
        v2 p1 = make_v2((f32)controller.mouse.pos.x, (f32)controller.mouse.pos.y);
        v2 direction = direction_v2(p0, p1);

        f32 rad = rad_from_dir(direction);
        f32 deg = deg_from_dir(direction);
    }
    if(event.type == KEYBOARD){
        if(event.key_pressed){
            if(event.keycode == KeyCode_ESCAPE){
                should_quit = true;
            }
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
        controller.mouse.pos = event.mouse_pos;
        controller.mouse.dx = event.mouse_dx;
        controller.mouse.dy = event.mouse_dy;
    }
    if(event.type == KEYBOARD){
        if(event.key_pressed){
            if(event.keycode == KeyCode_ONE){
                if(pm->game_mode != GameMode_FirstPerson){
                    pm->game_mode = GameMode_FirstPerson;

                    controller.mouse.dx = 0;
                    controller.mouse.dy = 0;

                    POINT center = {(window.width/2), (window.height/2)};
                    ClientToScreen(window.handle, &center);
                    show_cursor(false);
                }
            }
            if(event.keycode == KeyCode_TWO){
                if(pm->game_mode != GameMode_Editor){
                    pm->game_mode = GameMode_Editor;
                    show_cursor(true);
                }
            }
            if(event.keycode == KeyCode_THREE){
                if(pm->game_mode != GameMode_Game){
                    pm->game_mode = GameMode_Game;
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
    if(event.type == MOUSE){
        controller.mouse.pos = event.mouse_pos;
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

static void
update_game(Window* window, Memory* memory, Events* events){

    // NOTE: process events.
    while(!events_empty(events)){
        begin_timed_scope("events");
        Event event = events_next(events);

        bool handled;
        handled = handle_global_events(event);

        if(console_is_open()){
            handled = handle_console_events(event);
        }
        else{
            handled = handle_camera_events(event);
            handled = handle_controller_events(event);
        }
    }

    //----constant buffer----
    D3D11_MAPPED_SUBRESOURCE mapped_subresource;
    d3d_context->Map(d3d_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
    ConstantBuffer2D* constants = (ConstantBuffer2D*)mapped_subresource.pData;
    constants->screen_res = make_v2s32(window->width, window->height);
    d3d_context->Unmap(d3d_constant_buffer, 0);

    console_update();
    if(pm->game_mode == GameMode_Game && pm->score < WIN_SCORE){
        pm->spawn_t += clock.dt;
        if(pm->spawn_t >= 0.5f){
            pm->spawn_t = 0.0;

            v2 dim;
            dim.x = (f32)random_range_u32(150) + 50;
            dim.y = dim.x;
            u32 side = random_range_u32(3);

            v2 pos = {0, 0};
            f32 deg = 0;
            if(side == 0){
                pos.x = -200;
                pos.y = (f32)random_range_u32(SCREEN_HEIGHT - 1);
                deg = (f32)random_range_u32(180) - 90.0f;
            }
            if(side == 1){
                pos.x = SCREEN_WIDTH + 200;
                pos.y = (f32)random_range_u32(SCREEN_HEIGHT - 1);

                s32 sign = (s32)random_range_u32(1);
                if(sign == 0){
                    sign = -1;
                }
                deg = ((f32)random_range_u32(90) + 90.0f) * (f32)sign;
            }
            if(side == 2){
                pos.x = (f32)random_range_u32(SCREEN_WIDTH - 1);
                pos.y = -200;

                deg = (f32)random_range_u32(180);
            }
            if(side == 3){
                pos.x = (f32)random_range_u32(SCREEN_WIDTH - 1);
                pos.y = SCREEN_HEIGHT + 200;

                deg = (f32)random_range_u32(180) - 180;
            }
            Entity* e = add_asteroid(pm, &asteroid_shader_resource, pos, dim, deg);
        }

        if(controller.button[KeyCode_Y].held){
            wave_cursors[0].volume += (f32)clock.dt;
            if(wave_cursors[0].volume > 1.0f){
                wave_cursors[0].volume = 1.0f;
            }
        }
        if(controller.button[KeyCode_H].held){
            wave_cursors[0].volume -= (f32)clock.dt;
            if(wave_cursors[0].volume < 0.0f){
                wave_cursors[0].volume = 0.0f;
            }
        }
        if(controller.button[KeyCode_U].held){
            wave_cursors[1].volume += (f32)clock.dt;
            if(wave_cursors[1].volume > 1.0f){
                wave_cursors[1].volume = 1.0f;
            }
        }
        if(controller.button[KeyCode_J].held){
            wave_cursors[1].volume -= (f32)clock.dt;
            if(wave_cursors[1].volume < 0.0f){
                wave_cursors[1].volume = 0.0f;
            }
        }
        if(controller.button[KeyCode_I].held){
            wave_cursors[2].volume += (f32)clock.dt;
            if(wave_cursors[2].volume > 1.0f){
                wave_cursors[2].volume = 1.0f;
            }
        }
        if(controller.button[KeyCode_K].held){
            wave_cursors[2].volume -= (f32)clock.dt;
            if(wave_cursors[2].volume < 0.0f){
                wave_cursors[2].volume = 0.0f;
            }
        }
        if(controller.button[KeyCode_SIX].held){
            wave_cursors[0].at = 0;
        }
        if(controller.button[KeyCode_SEVEN].held){
            wave_cursors[1].at = 0;
        }
        if(controller.button[KeyCode_EIGHT].held){
            wave_cursors[2].at = 0;
        }

        for(s32 index = 0; index < array_count(pm->entities); ++index){
            begin_timed_scope("sim entities");
            Entity *e = pm->entities + index;

            switch(e->type){
                case EntityType_Ship:{
                    if(pm->ship_loaded){

                        // add bullet entity
                        if(controller.button[KeyCode_SPACEBAR].pressed){
                            add_bullet(pm, &circle_shader_resource, e->pos, make_v2(40, 8), e->deg);
                            audio_play(WaveAsset_bullet, 1.0f, false);
                            //audio_play_wav(bullet_sound);
                        }

                        // rotate ship
                        if(controller.button[KeyCode_RIGHT].held || controller.button[KeyCode_D].held){
                            f32 d = deg_from_dir(e->dir);
                            d += 200 * (f32)clock.dt;
                            e->dir = dir_from_deg(d);
                            e->deg = d;
                        }
                        if(controller.button[KeyCode_LEFT].held || controller.button[KeyCode_A].held){
                            f32 d = deg_from_dir(e->dir);
                            d -= 200 * (f32)clock.dt;
                            e->dir = dir_from_deg(d);
                            e->deg = d;
                        }

                        // increase ship velocity
                        if(controller.button[KeyCode_UP].held || controller.button[KeyCode_W].held){
                            e->velocity += (f32)clock.dt;
                        }
                        if(controller.button[KeyCode_DOWN].held || controller.button[KeyCode_S].held){
                            e->velocity -= (f32)clock.dt;
                        }
                        clamp_f32(0, 1, &e->velocity);

                        // move ship
                        e->pos.x += (e->dir.x * e->velocity * e->speed) * (f32)clock.dt;
                        e->pos.y += (e->dir.y * e->velocity * e->speed) * (f32)clock.dt;

                        Rect e_rect = make_rect(make_v2(e->pos.x - e->dim.w/2, e->pos.y - e->dim.h/2),
                                                make_v2(e->pos.x + e->dim.x/2, e->pos.y + e->dim.h/2));
                        for(s32 ast_idx = 0; ast_idx < array_count(pm->entities); ++ast_idx){
                            Entity *ast = pm->entities + ast_idx;

                            if(ast->type == EntityType_Asteroid){
                                Rect ast_rect = make_rect(make_v2(ast->pos.x - ast->dim.w/2, ast->pos.y - ast->dim.h/2),
                                                          make_v2(ast->pos.x + ast->dim.x/2, ast->pos.y + ast->dim.h/2));
                                if(rect_collides_rect(ast_rect, e_rect)){
                                    pm->lives -= 1;
                                    if(pm->lives){
                                        e->pos = make_v2(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
                                    }
                                    else{
                                        remove_entity(pm, e);
                                    }
                                    remove_entity(pm, ast);
                                    break;
                                }
                            }
                        }
                    }
                } break;
                case EntityType_Bullet:{
                    v2 dir = dir_from_deg(e->deg);
                    e->pos.x += (dir.x * e->velocity * e->speed) * (f32)clock.dt;
                    e->pos.y += (dir.y * e->velocity * e->speed) * (f32)clock.dt;

                    if((e->pos.x < 0 || e->pos.x > SCREEN_WIDTH) ||
                       (e->pos.y < 0 || e->pos.y > SCREEN_HEIGHT)){
                        remove_entity(pm, e);
                        break;
                    }

                    Rect e_rect = make_rect(make_v2(e->pos.x - e->dim.w/2, e->pos.y - e->dim.h/2),
                                            make_v2(e->pos.x + e->dim.x/2, e->pos.y + e->dim.h/2));
                    for(s32 ast_idx = 0; ast_idx < array_count(pm->entities); ++ast_idx){
                        Entity *ast = pm->entities + ast_idx;

                        if(ast->type == EntityType_Asteroid){
                            Rect ast_rect = make_rect(make_v2(ast->pos.x - ast->dim.w/2, ast->pos.y - ast->dim.h/2),
                                                      make_v2(ast->pos.x + ast->dim.x/2, ast->pos.y + ast->dim.h/2));
                            if(rect_collides_rect(ast_rect, e_rect)){
                                ast->health -= e->damage;
                                ast->color.r += 0.2f;
                                ast->color.g -= 0.4f;
                                ast->color.b -= 0.4f;
                                if(ast->health <= 0){
                                    pm->score += (u32)ast->dim.w;
                                    remove_entity(pm, ast);
                                }
                                remove_entity(pm, e);
                                break;
                            }
                        }
                    }

                // todo: flags for entities, no types
                // pass1: loop over entities, check flags, if controller flag, do controller stuff
                // pass2: do AI/asteroid movement for all entities
                } break;
                case EntityType_Asteroid:{
                    v2 dir = dir_from_deg(e->deg);
                    e->deg += e->rot_speed * (f32)clock.dt;
                    e->pos.x += (e->dir.x * e->velocity * e->speed) * (f32)clock.dt;
                    e->pos.y += (e->dir.y * e->velocity * e->speed) * (f32)clock.dt;

                    if((e->pos.x > 0 && e->pos.x < SCREEN_WIDTH) &&
                       (e->pos.y > 0 && e->pos.y < SCREEN_HEIGHT)){
                        e->in_play = true;
                    }
                    if((e->pos.x < 0 || e->pos.x > SCREEN_WIDTH) ||
                       (e->pos.y < 0 || e->pos.y > SCREEN_HEIGHT)){
                        if(e->in_play){
                            remove_entity(pm, e);
                            break;
                        }
                    }
                } break;
            }
        }
    }
}

#endif

