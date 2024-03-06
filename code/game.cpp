#ifndef GAME_C
#define GAME_C

static void
load_assets(Arena* arena, Assets* assets){
    assets->bitmaps[AssetID_Image] =  load_bitmap(arena, str8_literal("sprites\\image.bmp"));
    assets->bitmaps[AssetID_Ship] =   load_bitmap(arena, str8_literal("sprites\\ship2.bmp"));
    assets->bitmaps[AssetID_Tree] =   load_bitmap(arena, str8_literal("sprites\\tree00.bmp"));
    assets->bitmaps[AssetID_Circle] = load_bitmap(arena, str8_literal("sprites\\circle.bmp"));
    assets->bitmaps[AssetID_Bullet] = load_bitmap(arena, str8_literal("sprites\\bullet4.bmp"));
    assets->bitmaps[AssetID_Test] =   load_bitmap(arena, str8_literal("sprites\\test.bmp"));
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
    e->type = EntityType_None;
    pm->free_entities[++pm->free_entities_at] = e->index;
    pm->entities_count--;
    e->index = 0;
    e->generation = 0;
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
        e->origin = make_v2((pos.x + (pos.x + dim.w))/2, (pos.y + (pos.y + dim.h))/2);
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
        e->origin = make_v2((pos.x + (pos.x + dim.w))/2, (pos.y + (pos.y + dim.h))/2);
        e->texture = texture;
    }
    else{
        print("Failed to add entity: Quad\n");
    }
    return(e);
}

static Entity*
add_text(PermanentMemory* pm, Font font, String8 text, f32 x, f32 y, RGBA color){
    Entity* e = add_entity(pm, EntityType_Text);
    if(e){
        e->font = font;
        e->text = text;
        e->x = x;
        e->y = y;
        e->color = color;
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
        e->origin = make_v2((pos.x + (pos.x + dim.w))/2, (pos.y + (pos.y + dim.h))/2);
        e->speed = 400;
        e->velocity = 0;
        e->texture = texture;
    }
    else{
        print("Failed to add entity: Quad\n");
    }
    return(e);
}

static Entity*
add_bullet(PermanentMemory* pm, ID3D11ShaderResourceView** texture, v2 pos, v2 dim, f32 deg, RGBA color){
    Entity* e = add_entity(pm, EntityType_Bullet);
    if(e){
        e->dir = make_v2(1, 1);
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->deg = deg;
        e->speed = 200;
        e->velocity = 1;
        e->texture = texture;
    }
    else{
        print("Failed to add entity: Quad\n");
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
    String8 full_path = str8_path_append(scratch.arena, saves_dir, filename);

    File file = os_file_open(full_path, GENERIC_WRITE, CREATE_NEW);
    if(file.size){
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
    String8 full_path = str8_path_append(scratch.arena, saves_dir, filename);

    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    if(file.size){
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

                    //String8 ship_str = str8_literal("sprites\\ship_simple.bmp");
                    //Bitmap ship_image = load_bitmap(&pm->arena, ship_str);

                    pm->ship = ship;
                    pm->ship_loaded = true;
                } break;
            }
            offset += sizeof(Entity);
        }
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
            if(event.keycode == ESCAPE){
                should_quit = true;
            }
            if(event.keycode == Q_UPPER){
                g_angle_t = 0;
                p = 0;
            }
            if(event.keycode == W_UPPER){
                g_angle_t = 90;
                p = 0;
            }
            if(event.keycode == E_UPPER){
                g_angle_t = 180;
                p = 0;
            }
            if(event.keycode == R_UPPER){
                g_angle_t = 270;
                p = 0;
            }
            if(event.keycode == TILDE && !event.repeat){
                //console_t = 0;

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
    }
    return(false);
}

static bool
handle_camera_events(Event event){
    if(event.type == MOUSE){
        controller.mouse.pos = event.mouse_pos;
        controller.mouse.dx = event.mouse_dx;
        controller.mouse.dy = event.mouse_dy;
        return(true);
    }
    if(event.type == KEYBOARD){
        if(event.key_pressed){
            // TODO: log to screen these changes
            if(event.keycode == ONE){
                if(pm->game_mode != GameMode_FirstPerson){
                    pm->game_mode = GameMode_FirstPerson;

                    controller.mouse.dx = 0;
                    controller.mouse.dy = 0;

                    POINT center = {(window.width/2), (window.height/2)};
                    ClientToScreen(window.handle, &center);
                    show_cursor(false);
                }
            }
            if(event.keycode == TWO){
                if(pm->game_mode != GameMode_Editor){
                    pm->game_mode = GameMode_Editor;
                    show_cursor(true);
                }
            }
            if(event.keycode == THREE){
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
            if(event.keycode == SPACEBAR){
                if(!event.repeat){
                    controller.shoot.pressed = true;
                }
                controller.shoot.held = true;
                return(true);
            }
            if(event.keycode == Q_UPPER){
                if(!event.repeat){
                    controller.q.pressed = true;
                }
                controller.q.held = true;
                return(true);
            }
            if(event.keycode == E_UPPER){
                if(!event.repeat){
                    controller.e.pressed = true;
                }
                controller.e.held = true;
                return(true);
            }
            if(event.keycode == A_UPPER){
                if(!event.repeat){
                    controller.left.pressed = true;
                }
                controller.left.held = true;
                return(true);
            }
            if(event.keycode == D_UPPER){
                if(!event.repeat){
                    controller.right.pressed = true;
                }
                controller.right.held = true;
                return(true);
            }
            if(event.keycode == W_UPPER){
                if(!event.repeat){
                    controller.up.pressed = true;
                }
                controller.up.held = true;
                return(true);
            }
            if(event.keycode == S_UPPER){
                if(!event.repeat){
                    controller.down.pressed = true;
                }
                controller.down.held = true;
                return(true);
            }
        }
        else{
            if(event.keycode == SPACEBAR){
                controller.shoot.held = false;
                return(true);
            }
            if(event.keycode == Q_UPPER){
                controller.q.held = false;
                return(true);
            }
            if(event.keycode == E_UPPER){
                controller.e.held = false;
                return(true);
            }
            if(event.keycode == W_UPPER){
                controller.up.held = false;
                return(true);
            }
            if(event.keycode == A_UPPER){
                controller.left.held = false;
                return(true);
            }
            if(event.keycode == D_UPPER){
                controller.right.held = false;
                return(true);
            }
            if(event.keycode == S_UPPER){
                controller.down.held = false;
                return(true);
            }
        }
    }
    return(false);
}

static void
update_game(Window* window, Memory* memory, Events* events){

    // NOTE: process events.
    v3 camera_pos_vector = {0};
    v3 camera_forward_vector = {0};
    while(!events_empty(events)){
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

    f32 move_speed = 40;
    if(pm->game_mode == GameMode_Editor){
        //if(controller.right.held){
        //    second->pos.x += move_speed * (f32)clock.dt;
        //}
        //if(controller.left.held){
        //    second->pos.x -= move_speed * (f32)clock.dt;
        //}
        //if(controller.e.held){
        //    second->pos.y += move_speed * (f32)clock.dt;
        //}
        //if(controller.q.held){
        //    second->pos.y -= move_speed * (f32)clock.dt;
        //}
        //if(controller.up.held){
        //    second->pos.z += move_speed * (f32)clock.dt;
        //}
        //if(controller.down.held){
        //    second->pos.z -= move_speed * (f32)clock.dt;
        //}
    }
    if(pm->game_mode == GameMode_FirstPerson){

        // up down
        if(controller.e.held){
            f32 dy = (f32)(camera.move_speed * clock.dt);
            camera.pos.y += dy;
        }
        if(controller.q.held){
            f32 dy = (f32)(camera.move_speed * clock.dt);
            camera.pos.y -= dy;
        }

        // wasd
        if(controller.up.held){
            v3 result = (camera.forward  * camera.move_speed * (f32)clock.dt);
            camera.pos = camera.pos + result;
        }
        if(controller.down.held){
            v3 result = (camera.forward  * camera.move_speed * (f32)clock.dt);
            camera.pos = camera.pos - result;
        }
        if(controller.left.held){
            v3 result = (normalize_v3(cross_product_v3(camera.forward, make_v3(0, 1, 0))) * camera.move_speed * (f32)clock.dt);
            camera.pos = camera.pos + result;
        }
        if(controller.right.held){
            v3 result = (normalize_v3(cross_product_v3(camera.forward, make_v3(0, 1, 0))) * camera.move_speed * (f32)clock.dt);
            camera.pos = camera.pos - result;
        }

        POINT center = {(SCREEN_WIDTH/2), (SCREEN_HEIGHT/2)};
        ClientToScreen(window->handle, &center);
        SetCursorPos(center.x, center.y);
        update_camera(controller.mouse.dx, controller.mouse.dy, (f32)clock.dt);
    }

    console_update_openess();

    XMVECTOR camera_pos = {camera.pos.x, camera.pos.y, camera.pos.z};
    XMVECTOR camera_forward = {camera.forward.x, camera.forward.y, camera.forward.z};
    XMVECTOR camera_up = {camera.up.x, camera.up.y, camera.up.z};
    XMMATRIX view_matrix = XMMatrixLookAtLH(camera_pos, camera_pos + camera_forward, camera_up);
    XMMATRIX perspective_matrix = XMMatrixPerspectiveFovLH(PI_f32*0.25f, (f32)((f32)SCREEN_WIDTH/(f32)SCREEN_HEIGHT), 1.0f, 1000.0f);

    // note: set constant buffer
    //D3D11_MAPPED_SUBRESOURCE mapped_subresource;
    //d3d_context->Map(d3d_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
    //ConstantBuffer* constants = (ConstantBuffer*)mapped_subresource.pData;
    //constants->view = view_matrix;
    //constants->projection = perspective_matrix;
    //d3d_context->Unmap(d3d_constant_buffer, 0);
    //d3d_context->VSSetConstantBuffers(0, 1, &d3d_constant_buffer);

    //----constant buffer----
    D3D11_MAPPED_SUBRESOURCE mapped_subresource;
    d3d_context->Map(d3d_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
    ConstantBuffer2D* constants = (ConstantBuffer2D*)mapped_subresource.pData;
    constants->screen_res = make_v2s32(window->width, window->height);
    d3d_context->Unmap(d3d_constant_buffer, 0);

    for(s32 index = 0; index < array_count(pm->entities); ++index){
        Entity *e = pm->entities + index;

        switch(e->type){
            case EntityType_Ship:{
                if(pm->ship_loaded){
                    Entity* ship = pm->ship;

                    // add bullet entity
                    if(controller.shoot.pressed){
                        add_bullet(pm, &circle_shader_resource, ship->pos, ship->dim, ship->deg);
                    }

                    // rotate ship
                    if(controller.right.held){
                        ship->deg += 200 * (f32)clock.dt;
                    }
                    if(controller.left.held){
                        ship->deg -= 200 * (f32)clock.dt;
                    }

                    // increase ship velocity
                    if(controller.up.held){
                        ship->velocity += (f32)clock.dt;
                    }
                    if(controller.down.held){
                        ship->velocity -= (f32)clock.dt;
                    }
                    clamp_f32(0, 1, &ship->velocity);

                    // move ship
                    v2 dir = dir_from_deg(ship->deg);
                    ship->pos.x += (dir.x * ship->velocity * ship->speed) * (f32)clock.dt;
                    ship->pos.y += (dir.y * ship->velocity * ship->speed) * (f32)clock.dt;
                }
            } break;
            case EntityType_Bullet:{
                v2 dir = dir_from_deg(e->deg);
                e->pos.x += (dir.x * e->velocity * e->speed) * (f32)clock.dt;
                e->pos.y += (dir.y * e->velocity * e->speed) * (f32)clock.dt;

                if((e->pos.x < 0 || e->pos.x > SCREEN_WIDTH) ||
                   (e->pos.y < 0 || e->pos.y > SCREEN_HEIGHT)){
                    remove_entity(pm, e);
                }
            } break;
        }
    }
}

#endif

