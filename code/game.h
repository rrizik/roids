#ifndef GAME_H
#define GAME_H


static Font global_font = {0};



typedef enum GameMode{
    GameMode_FirstPerson,
    GameMode_Editor,
    GameMode_Game,
} GameMode;

typedef enum AssetID{
    AssetID_Image,
    AssetID_Ship,
    AssetID_Tree,
    AssetID_Circle,
    AssetID_Bullet,
    AssetID_Test,

    AssetID_count,
} AssetID;

typedef struct Assets{
    Bitmap bitmaps[AssetID_count];
    Bitmap image_image;
    Bitmap ship_image;
    Bitmap tree_image;
    Bitmap circle_image;
    Bitmap bullet_image;
    Bitmap test_image;
} Assets;

static void
load_assets(Arena* arena, Assets* assets){
    assets->bitmaps[AssetID_Image] = load_bitmap(str8_literal("sprites\\image.bmp"));
    assets->bitmaps[AssetID_Ship] = load_bitmap(str8_literal("sprites\\ship.bmp"));
    assets->bitmaps[AssetID_Tree] = load_bitmap(str8_literal("sprites\\tree00.bmp"));
    assets->bitmaps[AssetID_Circle] = load_bitmap(str8_literal("sprites\\circle.bmp"));
    assets->bitmaps[AssetID_Bullet] = load_bitmap(str8_literal("sprites\\bullet4.bmp"));
    assets->bitmaps[AssetID_Test] = load_bitmap(str8_literal("sprites\\test.bmp"));
}

static Bitmap*
get_bitmap(Assets* assets, AssetID id){
    Bitmap* bitmap = assets->bitmaps + id;
    return(bitmap);
}

#define ENTITIES_MAX 100
typedef struct PermanentMemory{
    Arena arena;
    u32 game_mode;

    u32 generation[ENTITIES_MAX];
    u32 free_entities[ENTITIES_MAX];
    u32 free_entities_at;

    Entity entities[ENTITIES_MAX];
    u32 entities_count;

    Entity* texture;
    Entity* circle;
    Entity* basis;
    Entity* ship;
    Bitmap tree;
    bool ship_loaded;

} PermanentMemory, State;
global PermanentMemory* pm;

typedef struct TransientMemory{
    Arena arena;
    Arena *frame_arena;
    Arena *render_command_arena;

    Assets assets;
} TransientMemory;
global TransientMemory* tm;

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
add_pixel(PermanentMemory* pm, Rect rect, RGBA color){
    Entity* e = add_entity(pm, EntityType_Pixel);
    e->rect = rect;
    e->color = color;
    return(e);
}

static Entity*
add_segment(PermanentMemory* pm, v2 p0, v2 p1, RGBA color){
    Entity* e = add_entity(pm, EntityType_Segment);
    e->color = color;
    e->p0 = p0;
    e->p1 = p1;
    return(e);
}

static Entity*
add_ray(PermanentMemory* pm, Rect rect, v2 direction, RGBA color){
    Entity* e = add_entity(pm, EntityType_Ray);
    e->rect = rect;
    e->color = color;
    e->direction = direction;
    return(e);
}

static Entity*
add_line(PermanentMemory* pm, Rect rect, v2 direction, RGBA color){
    Entity* e = add_entity(pm, EntityType_Line);
    e->rect = rect;
    e->color = color;
    e->direction = direction;
    return(e);
}

static Entity*
add_rect(PermanentMemory* pm, Rect rect, RGBA color, s32 bsize = 0, RGBA bcolor = {0, 0, 0, 0}){
    Entity* e = add_entity(pm, EntityType_Rect);
    e->rect =  rect;
    e->color = color;
    e->border_size =  bsize;
    e->border_color = bcolor;
    return(e);
}

static Entity*
add_basis(PermanentMemory* pm, v2 origin, v2 x_axis, v2 y_axis, Bitmap* texture, RGBA color = {0, 0, 0, 1}){
    Entity* e = add_entity(pm, EntityType_Bases);
    e->origin = origin;
    e->x_axis = x_axis;
    e->y_axis = y_axis;
    e->color = color;
    e->texture = texture;
    return(e);
}

static Entity*
add_ship(PermanentMemory* pm, v2 origin, v2 x_axis, v2 y_axis, Bitmap* texture, RGBA color = {0, 0, 0, 1}){
    Entity* e = add_entity(pm, EntityType_Ship);
    e->origin = origin;
    e->x_axis = x_axis;
    e->y_axis = y_axis;
    e->color = color;
    e->texture = texture;
    e->direction = make_v2(0, 1);
    e->rad = dir_to_rad(e->direction);
    e->speed = 250;
    //e->scale = 50;
    pm->ship_loaded = true; // TODO: get rid of
    return(e);
}

static Entity*
add_bullet(PermanentMemory* pm, v2 origin, v2 x_axis, v2 y_axis, Bitmap* texture, RGBA color = {0, 0, 0, 1}){
    Entity* e = add_entity(pm, EntityType_Bullet);
    e->origin = origin;
    e->x_axis = x_axis;
    e->y_axis = y_axis;
    e->color = color;
    e->texture = texture;
    e->direction = make_v2(0, 1);
    e->rad = dir_to_rad(e->direction);
    e->speed = 250;
    //e->scale = 10;
    return(e);
}

static Entity*
add_cube(PermanentMemory* pm, Bitmap* texture, v3 pos, v3 angle, v3 scale, u32 index){
    Entity* e = add_entity(pm, EntityType_Cube);
    e->index = index;
    e->pos = pos;
    e->angle = angle;
    e->scale = scale;
    e->texture = texture;
    return(e);
}

static Entity*
add_player(PermanentMemory* pm, Bitmap* texture, v3 pos, v3 angle, v3 scale, u32 index){
    Entity* e = add_entity(pm, EntityType_Player);
    e->index = index;
    e->pos = pos;
    e->angle = angle;
    e->scale = scale;
    e->texture = texture;
    return(e);
}

static Entity*
add_box(PermanentMemory* pm, Rect rect, RGBA color){
    Entity* e = add_entity(pm, EntityType_Box);
    e->rect = rect;
    e->color = color;
    return(e);
}

static Entity*
add_quad(PermanentMemory* pm, v2 p0, v2 p1, v2 p2, v2 p3, RGBA color, bool fill){
    Entity* e = add_entity(pm, EntityType_Quad);
    e->color = color;
    e->p0 = p0;
    e->p1 = p1;
    e->p2 = p2;
    e->p3 = p3;
    e->fill = fill;
    return(e);
}

static Entity*
add_triangle(PermanentMemory *pm, v2 p0, v2 p1, v2 p2, RGBA color, bool fill){
    Entity* e = add_entity(pm, EntityType_Triangle);
    e->p0 = p0;
    e->p1 = p1;
    e->p2 = p2;
    e->color = color;
    e->fill = fill;
    return(e);
}

static Entity*
add_circle(PermanentMemory *pm, Rect rect, u8 rad, RGBA color, bool fill){
    Entity* e = add_entity(pm, EntityType_Circle);
    e->rect = rect;
    e->color = color;
    e->fill = fill;
    e->rad = rad;
    return(e);
}

static Entity*
add_bitmap(PermanentMemory* pm, v2 pos, Bitmap* texture){
    Entity* e = add_entity(pm, EntityType_Bitmap);
    e->rect = make_rect(pos.x, pos.y, 0, 0);
    e->texture = texture;
    return(e);
}

static void
entities_clear(PermanentMemory* pm){
    pm->free_entities_at = ENTITIES_MAX - 1;
    for(u32 i = pm->free_entities_at; i <= pm->free_entities_at; --i){
        pm->free_entities[i] = pm->free_entities_at - i;
        pm->generation[i] = 0;
    }
    pm->entities_count = 0;
}

static void
serialize_data(PermanentMemory* pm, String8 filename){
    File file = os_file_open(filename, GENERIC_WRITE, CREATE_NEW);
    assert_fh(file);

    os_file_write(&file, pm->entities, sizeof(Entity) * ENTITIES_MAX);
    os_file_close(&file);
}

static void
deserialize_data(PermanentMemory* pm, String8 filename){
    File file = os_file_open(filename, GENERIC_READ, OPEN_EXISTING);
    assert_fh(file);
    String8 data = os_file_read(&pm->arena, &file);

    entities_clear(pm);
    u32 offset = 0;
    while(offset < data.size){
        Entity* e = (Entity*)(data.str + offset);
        switch(e->type){
            case EntityType_Ship:{
                Entity* ship = add_entity(pm, EntityType_Ship);
                *ship = *e;

                String8 ship_str = str8_literal("sprites\\ship_simple.bmp");
                Bitmap ship_image = load_bitmap(ship_str);
                ship->texture = get_bitmap(&tm->assets, AssetID_Ship);

                pm->ship = ship;
                pm->ship_loaded = true;
            } break;
            case EntityType_Rect:{
                add_rect(pm, e->rect, e->color, e->border_size, e->border_color);
            } break;
        }
        offset += sizeof(*e);
    }
}

#include "console.h"

static bool
handle_global_event(Event event){
    if(event.type == MOUSE){
        controller.mouse.pos = event.mouse_pos;
        controller.mouse.dx = event.mouse_dx;
        controller.mouse.dy = event.mouse_dy;
    }
    if(event.type == QUIT){
        should_quit = true;
    }
    if(event.type == KEYBOARD){
        if(event.key_pressed){
            if(event.keycode == ESCAPE){
                should_quit = true;
            }
            if(event.keycode == TILDE && !event.repeat){
                console_t = 0;

                if(event.shift_pressed){
                    if(console.state == OPEN_BIG){
                        console.state = CLOSED;
                    }
                    else{ console.state = OPEN_BIG; }
                }
                else{
                    if(console.state == OPEN || console.state == OPEN_BIG){
                        console.state = CLOSED;
                    }
                    else{ console.state = OPEN; }

                }
                return(true);
            }
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
    if(event.type == KEYBOARD){
        if(event.key_pressed){
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

//static String8 tree_str   = str8_literal("tree00.bmp");
//static String8 image_str  = str8_literal("image.bmp");
//static String8 test_str   = str8_literal("test3.bmp");
//static String8 circle_str = str8_literal("circle.bmp");
//static String8 ship_str   = str8_literal("ship_simple.bmp");
//static String8 bullet_str = str8_literal("bullet4.bmp");

static f32 angle = 0;
static void
//update_game(Memory* memory, RenderBuffer* render_buffer, Events* events, Clock* clock){
update_game(Window* window, Memory* memory, Events* events, Clock* clock){
    assert(sizeof(PermanentMemory) < memory->permanent_size);
    assert(sizeof(TransientMemory) < memory->transient_size);
    pm = (PermanentMemory*)memory->permanent_base;
    tm = (TransientMemory*)memory->transient_base;


    //arena_free(render_buffer->render_command_arena);
    //push_clear_color(render_buffer->render_command_arena, BLACK);
    //Arena* render_command_arena = render_buffer->render_command_arena;
    if(!memory->initialized){
        pm->game_mode = GameMode_Game;
        show_cursor(false);

        init_camera();

        init_arena(&pm->arena, (u8*)memory->permanent_base + sizeof(PermanentMemory), memory->permanent_size - sizeof(PermanentMemory));
        init_arena(&tm->arena, (u8*)memory->transient_base + sizeof(TransientMemory), memory->transient_size - sizeof(TransientMemory));

        tm->render_command_arena = push_arena(&tm->arena, MB(16));
        tm->frame_arena = push_arena(&tm->arena, MB(100));

        // setup free entities array (max to 0)
        entities_clear(pm);

        //pm->sprites_dir = str8_path_append(&pm->arena, path_data, str8_literal("sprites"));
        //pm->fonts_dir   = str8_path_append(&pm->arena, path_data, str8_literal("fonts"));
        //pm->saves_dir   = str8_path_append(&pm->arena, path_data, str8_literal("saves"));

        // basis test
        //d3d_init_texture(ship_image);


        load_assets(&tm->arena, &tm->assets);
        //Bitmap ship_image = load_bitmap(&pm->arena, pm->sprites_dir, ship_str);
        //Bitmap aa = stb_load_image(pm->sprites_dir, circle_str);
        //Bitmap ship_image = stb_load_image(pm->sprites_dir, ship_str);
        //Bitmap test_image0 = stb_load_image(pm->sprites_dir, test_str);
        //Bitmap test_image1 = stb_load_image(pm->sprites_dir, test_str);
        //Bitmap test_image1 = load_bitmap(&pm->arena, pm->sprites_dir, test_str);

		v2 origin = make_v2((f32)resolution.x/2, (f32)resolution.y/2);
        v2 direction = make_v2(0, 1);
		//v2 x_axis = 100.0f * make_v2(cos_f32(dir_to_rad(direction)), sin_f32(dir_to_rad(direction)));
		v2 x_axis = make_v2(1, 1);
		v2 y_axis = make_v2(-x_axis.y, x_axis.x);
        //pm->ship = add_basis(pm, origin, x_axis, y_axis, ship_image, {0, 0, 0, 0});
        //add_basis(pm, origin, x_axis, y_axis, tree_image, {0, 0, 0, 0});
        //add_basis(pm, origin, x_axis, y_axis, circle_image, {0, 0, 0, 0});
        //add_basis(pm, origin, x_axis, y_axis, test_image0, {0, 0, 0, 0});
        //pm->ship = add_ship(pm, make_v2(100, 100), x_axis, y_axis, ship_image, {0, 0, 0, 0});
        Entity* bullet = add_bullet(pm, origin, x_axis, y_axis, get_bitmap(&tm->assets, AssetID_Bullet), {0, 0, 0, 0});
        //add_rect(pm, rect_screen_to_pixel(make_rect(.1, .5f, .2, .6), resolution), MAGENTA);
        //add_rect(pm, rect_screen_to_pixel(make_rect(.3, .5f, .4, .6), resolution), MAGENTA, 4, GREEN);
        //add_rect(pm, rect_screen_to_pixel(make_rect(.5, .5f, .6, .6), resolution), MAGENTA, 0, BLUE);
        //add_rect(pm, rect_screen_to_pixel(make_rect(.7, .5f, .8, .6), resolution), MAGENTA, -20000, TEAL);

        first = add_cube(pm, get_bitmap(&tm->assets, AssetID_Test), make_v3(40.0f, 0.0f, 200.0f), make_v3(0.0f, 0.0f, 0.0f), make_v3(0.5f, 0.5f, 0.5f), 120);
        second = add_player(pm, get_bitmap(&tm->assets, AssetID_Ship), make_v3(-40.0f, 0.0f, 200.0f), make_v3(0.0f, 0.0f, 0.0f), make_v3(0.2f, 0.2f, 0.2f), 121);
        third = add_player(pm, get_bitmap(&tm->assets, AssetID_Ship), make_v3(0.0f, 0.0f, -200.0f), make_v3(0.0f, 0.0f, 0.0f), make_v3(0.2f, 0.2f, 0.2f), 121);
        //Inconsolata-Regular
        Bitmap inconsolate[128];

        //String8 incon = str8_literal("MatrixSans-Regular.ttf");
        //String8 incon = str8_literal("MatrixSans-Video.ttf");
        //String8 incon = str8_literal("Rock Jack Writing.ttf");
        //String8 incon = str8_literal("Inconsolata-Regular.ttf");
        String8 roboto = str8_literal("fonts\\Roboto-Regular.ttf");
        String8 golos = str8_literal("fonts\\GolosText-Regular.ttf");
        String8 arial = str8_literal("fonts\\arial.ttf");
        String8 incon = str8_literal("fonts\\consola.ttf");
        global_font.name = str8_literal("fonts\\GolosText-Regular.ttf");
        global_font.size = 24;
        global_font.color = WHITE;
        bool succeed = load_font_ttf(&pm->arena, str8_literal("fonts\\GolosText-Regular.ttf"), &global_font);
        assert(succeed);
        load_font_glyphs(&pm->arena, &global_font, RED);

        init_console(pm);
        init_commands();

        memory->initialized = true;
    }
    angle += (f32)clock->dt;
    //second->pos.z = cos_f32(angle);


    // NOTE: process events.
    v3 camera_pos_vector = {0};
    v3 camera_forward_vector = {0};
    while(!events_empty(events)){
        Event event = events_next(events);

        bool handled;
        handled = handle_global_event(event);

        if(console_is_open()){
            handled = handle_console_event(event);
        }
        else{
            handled = handle_controller_events(event);
        }
    }

    f32 move_speed = 40;
    if(pm->game_mode == GameMode_Editor){
        if(controller.right.held){
            second->pos.x += move_speed * (f32)clock->dt;
        }
        if(controller.left.held){
            second->pos.x -= move_speed * (f32)clock->dt;
        }
        if(controller.e.held){
            second->pos.y += move_speed * (f32)clock->dt;
        }
        if(controller.q.held){
            second->pos.y -= move_speed * (f32)clock->dt;
        }
        if(controller.up.held){
            second->pos.z += move_speed * (f32)clock->dt;
        }
        if(controller.down.held){
            second->pos.z -= move_speed * (f32)clock->dt;
        }
    }
    if(pm->game_mode == GameMode_FirstPerson){

        // up down
        if(controller.e.held){
            f32 dy = (f32)(camera.move_speed * clock->dt);
            camera.pos.y += dy;
        }
        if(controller.q.held){
            f32 dy = (f32)(camera.move_speed * clock->dt);
            camera.pos.y -= dy;
        }

        // wasd
        if(controller.up.held){
            v3 result = (camera.forward  * camera.move_speed * (f32)clock->dt);
            camera.pos = camera.pos + result;
        }
        if(controller.down.held){
            v3 result = (camera.forward  * camera.move_speed * (f32)clock->dt);
            camera.pos = camera.pos - result;
        }
        if(controller.left.held){
            v3 result = (normalized_v3(cross_product_v3(camera.forward, (v3){0, 1, 0})) * camera.move_speed * (f32)clock->dt);
            camera.pos = camera.pos + result;
        }
        if(controller.right.held){
            v3 result = (normalized_v3(cross_product_v3(camera.forward, (v3){0, 1, 0})) * camera.move_speed * (f32)clock->dt);
            camera.pos = camera.pos - result;
        }

        POINT center = {(SCREEN_WIDTH/2), (SCREEN_HEIGHT/2)};
        ClientToScreen(window->handle, &center);
        SetCursorPos(center.x, center.y);
        update_camera(controller.mouse.dx, controller.mouse.dy, (f32)clock->dt);
    }

    //if(pm->ship_loaded){
    //    Entity* ship = pm->ship;
    //    // rotate ship
    //    if(controller.right.held){
    //        ship->rad -= 2 * (f32)clock->dt;
    //        second->pos.x += 1 * (f32)clock->dt;
    //    }
    //    if(controller.left.held){
    //        ship->rad += 2 * (f32)clock->dt;
    //        second->pos.x -= 1 * (f32)clock->dt;
    //    }

    //    // increase ship velocity
    //    if(controller.up.held){
    //        ship->velocity += (f32)clock->dt;
    //    }
    //    if(controller.down.held){
    //        ship->velocity -= (f32)clock->dt;
    //    }
    //    clamp_f32(0, 1, &ship->velocity);

    //    // move ship
    //    ship->direction = rad_to_dir(ship->rad);
    //    ship->origin.x += (ship->direction.x * ship->velocity * ship->speed) * (f32)clock->dt;
    //    ship->origin.y += (ship->direction.y * ship->velocity * ship->speed) * (f32)clock->dt;
    //    //print("x: %f - y: %f - v: %f - a: %f\n", ship->direction.x, ship->direction.y, ship->velocity, ship->rad);
    //}
    update_console();

    XMVECTOR camera_pos = (XMVECTOR){camera.pos.x, camera.pos.y, camera.pos.z};
    XMVECTOR camera_forward = (XMVECTOR){camera.forward.x, camera.forward.y, camera.forward.z};
    XMVECTOR camera_up = (XMVECTOR){camera.up.x, camera.up.y, camera.up.z};
    XMMATRIX view_matrix = XMMatrixLookAtLH(camera_pos, camera_pos + camera_forward, camera_up);
    XMMATRIX perspective_matrix = XMMatrixPerspectiveFovLH(PI_f32*0.25f, (f32)((f32)SCREEN_WIDTH/(f32)SCREEN_HEIGHT), 1.0f, 1000.0f);

    D3D11_MAPPED_SUBRESOURCE mapped_subresource;
    d3d_context->Map(d3d_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);

    ConstantBuffer* constants = (ConstantBuffer*)mapped_subresource.pData;
    constants->view = view_matrix;
    constants->projection = perspective_matrix;
    d3d_context->Unmap(d3d_constant_buffer, 0);

    d3d_context->VSSetConstantBuffers(0, 1, &d3d_constant_buffer);



    f32 aspect_ratio = (f32)SCREEN_WIDTH / (f32)SCREEN_HEIGHT;
    u32 c = array_count(pm->entities) - 1;
    for(u32 entity_index = (u32)pm->free_entities_at; entity_index < array_count(pm->entities); ++entity_index){
        Entity *e = pm->entities + pm->free_entities[entity_index];

        switch(e->type){
            case EntityType_Cube:{
                e->angle.z += (f32)clock->dt;
                e->angle.y += (f32)clock->dt;

                InstanceData* instance = cube_instances + ((c - 1) - entity_index);
                instance->transform =
                    XMMatrixRotationX(e->angle.x) *
                    XMMatrixRotationY(e->angle.y) *
                    XMMatrixRotationZ(e->angle.z) *
                    XMMatrixScaling(e->scale.x, e->scale.y, e->scale.z) *
                    XMMatrixTranslation(e->pos.x, e->pos.y, e->pos.z);
            } break;
            case EntityType_Player:{
                e->angle.z += (f32)clock->dt;
                e->angle.x += (f32)clock->dt;

                InstanceData* instance = cube_instances + ((c - 1) - entity_index);
                instance->transform =
                    XMMatrixRotationX(e->angle.x) *
                    XMMatrixRotationY(e->angle.y) *
                    XMMatrixRotationZ(e->angle.z) *
                    XMMatrixScaling(e->scale.x, e->scale.y, e->scale.z) *
                    XMMatrixTranslation(e->pos.x, e->pos.y, e->pos.z);
            } break;
        }
    }

    if(console_is_visible()){
        //draw_console();
        //push_console(tm->render_command_arena);
    }
    String8 text = str8_literal("get! This is my program.\nIt renders fonts.\nHere is some dummy text 123.\nMore Dummy Text ONETWOTHREE\nEND OF DUMMY_TEXT_TEST.H OK");
    //String8 text   = str8_literal("g");
    String8 strings[] = {
        str8_literal("Console:"),
        str8_literal("  - Cursor - left/right movement. up/down through history. home/end."),
        str8_literal("  - Editing - start, middle, end of cursor pos"),
        str8_literal("  - Commands - console commands like (help, exit, add, save, load)"),
        str8_literal("                           - save/load will serialize/deserialize entity data"),
    };
    //push_text_array(render_command_arena, make_v2(((f32)resolution.x/2.0f) - 50, ((f32)resolution.h/2.0f) -250), &global_font, strings, array_count(strings));
    //push_text(render_command_arena, make_v2(100, 200), &global_font, text);

    //String8 one   = str8_literal("get! This is my program.");
    //String8 two   = str8_literal("It renders fonts.");
    //String8 three = str8_literal("Here is some dummy text 123.");
    //String8 four  = str8_literal("More Dummy Text ONETWOTHREE");
    //String8 five  = str8_literal("END OF DUMMY_TEXT_TEST.H OK");
    //push_text(render_command_arena, make_v2(0, resolution.h - 50), &global_font, one);
    //push_text(render_command_arena, make_v2(0, resolution.h - 100), &global_font, two);
    //push_text(render_command_arena, make_v2(0, resolution.h - 150), &global_font, three);
    //push_text(render_command_arena, make_v2(0, resolution.h - 200), &global_font, four);
    //push_text(render_command_arena, make_v2(0, resolution.h - 250), &global_font, five);
    //push_segment(render_command_arena, make_v2(0, resolution.h - 50), make_v2(700, resolution.h - 50), RED);
    //push_segment(render_command_arena, make_v2(0, resolution.h - 100), make_v2(700, resolution.h - 100), RED);
    //push_segment(render_command_arena, make_v2(0, resolution.h - 150), make_v2(700, resolution.h - 150), RED);
    //push_segment(render_command_arena, make_v2(0, resolution.h - 200), make_v2(700, resolution.h - 200), RED);
    //push_segment(render_command_arena, make_v2(0, resolution.h - 250), make_v2(700, resolution.h - 250), RED);


    //draw_pixel(render_buffer, make_v2(1, 1), RED);
    //draw_pixel(render_buffer, make_v2(0, 0), RED);
    //push_rect(render_command_arena, make_rect(20, 20, render_buffer->width + 10, render_buffer->height + 10), RED);
    //push_rect(render_command_arena, Rect rect, RGBA color, s32 border_size = 0, RGBA border_color = {0, 0, 0, 0}){
    //draw_rect_slow(render_buffer, make_v2(50, 50), make_v2(100, 0), make_v2(0, 100), GREEN);
    //draw_rect_slow2(render_buffer, make_v2(50, 50), make_v2(100, 0), make_v2(0, 100), YELLOW);
    String8 s = str8_literal("Rafik hahahah LOLOLOLOL");
    //draw_string(render_buffer, make_v2(500, 300), s, 0xF8DB5E);
    //draw_bitmap(render_buffer, make_v2(100, 100), &pm->tree);

    clear_controller_pressed(&controller);
    arena_free(&tm->arena);
}

#endif

