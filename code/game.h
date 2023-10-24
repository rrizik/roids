#ifndef GAME_H
#define GAME_H


static Font global_font = {0};

static RGBA RED =     {1.0f, 0.0f, 0.0f,  1.0f};
static RGBA GREEN =   {0.0f, 1.0f, 0.0f,  1.0f};
static RGBA BLUE =    {0.0f, 0.0f, 1.0f,  1.0f};
static RGBA MAGENTA = {1.0f, 0.0f, 1.0f,  1.0f};
static RGBA TEAL =    {0.0f, 1.0f, 1.0f,  1.0f};
static RGBA PINK =    {0.92f, 0.62f, 0.96f, 1.0f};
static RGBA YELLOW =  {0.9f, 0.9f, 0.0f,  1.0f};
static RGBA ORANGE =  {1.0f, 0.5f, 0.15f,  1.0f};
static RGBA DARK_GRAY =   {0.5f, 0.5f, 0.5f,  1.0f};
static RGBA LIGHT_GRAY =   {0.8f, 0.8f, 0.8f,  1.0f};
static RGBA WHITE =   {1.0f, 1.0f, 1.0f,  1.0f};
static RGBA BLACK =   {0.0f, 0.0f, 0.0f,  1.0f};
static RGBA ARMY_GREEN =   {0.25f, 0.25f, 0.23f,  1.0f};
static RGBA BACKGROUND_COLOR = {0.2f, 0.29f, 0.29f, 1.0f};


enum GameMode{
    GameMode_FirstPerson,
    GameMode_Editor,
    GameMode_Game
};
#define ENTITIES_MAX 100
typedef struct PermanentMemory{
    Arena arena;
    u32 game_mode;

    u32 generation[ENTITIES_MAX];
    u32 free_entities[ENTITIES_MAX];
    u32 free_entities_at;

    Entity entities[ENTITIES_MAX];
    u32 entities_count;

    Mesh meshes[100];

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
} TransientMemory;
global TransientMemory* tm;

static void
init_meshes(Mesh* meshes){
    // CUBE
    Mesh* cube_mesh = (Mesh*)(meshes + EntityType_Cube);
    cube_mesh->vertex_offset = 0;
    cube_mesh->vertex_stride = sizeof(Vertex);
    cube_mesh->vertex_count = array_count(cube);
    cube_mesh->verticies = cube;
    //d3d_init_vertex_buffers(cube_mesh, cube);

    cube_mesh->index_stride = sizeof(u32);
    cube_mesh->index_count = array_count(cube_indicies);
    cube_mesh->indicies = cube_indicies;
    //d3d_init_index_buffer(cube_mesh, cube_indicies);
    ////////////////////////////////
}

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
add_basis(PermanentMemory* pm, v2 origin, v2 x_axis, v2 y_axis, Bitmap texture, RGBA color = {0, 0, 0, 1}){
    Entity* e = add_entity(pm, EntityType_Bases);
    e->origin = origin;
    e->x_axis = x_axis;
    e->y_axis = y_axis;
    e->color = color;
    e->texture = texture;
    return(e);
}

static Entity*
add_ship(PermanentMemory* pm, v2 origin, v2 x_axis, v2 y_axis, Bitmap texture, RGBA color = {0, 0, 0, 1}){
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
add_bullet(PermanentMemory* pm, v2 origin, v2 x_axis, v2 y_axis, Bitmap texture, RGBA color = {0, 0, 0, 1}){
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
add_cube(PermanentMemory* pm, Bitmap texture, v3 pos, v3 angle, v3 scale, u32 index){
    Entity* e = add_entity(pm, EntityType_Cube);
    e->index = index;
    e->pos = pos;
    e->angle = angle;
    e->scale = scale;
    e->texture = texture;
    return(e);
}

static Entity*
add_player(PermanentMemory* pm, Bitmap texture, v3 pos, v3 angle, v3 scale, u32 index){
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
add_bitmap(PermanentMemory* pm, v2 pos, Bitmap texture){
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
    File file = os_file_open(path_saves, filename, 1);
    assert_fh(file);

    os_file_write(&file, pm->entities, sizeof(Entity) * ENTITIES_MAX);
    os_file_close(&file);
}

static void
deserialize_data(PermanentMemory* pm, String8 filename){
    File file = os_file_open(path_saves, filename);
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

                String8 ship_str = str8_literal("\\ship_simple.bmp");
                Bitmap ship_image = load_bitmap(&tm->arena, path_saves, ship_str);
                ship->texture = ship_image;

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
    if(event.type == QUIT){
        print("quiting\n");
        should_quit = true;
    }
    if(event.type == KEYBOARD){
        if(event.key_pressed){
            if(event.keycode == ESCAPE){
                print("quiting\n");
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
            if(event.keycode == ONE){
                if(event.shift_pressed){
                    pm->game_mode = GameMode_FirstPerson | GameMode_Editor;
                    print("FP EDITOR\n");
                }
                else{
                    pm->game_mode = GameMode_Editor;
                    print("EDITOR\n");
                }
            }
            if(event.keycode == TWO){
                pm->game_mode = GameMode_Game;
                print("GAME MODE\n");
            }
        }
    }
    return(false);
}


static bool
handle_controller_events(Event event){
    if(event.type == MOUSE){
        controller.mouse_pos = event.mouse_pos;
    }
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

static f32 angle = 0;
static Entity* first;
static Entity* second;
static void
//update_game(Memory* memory, RenderBuffer* render_buffer, Events* events, Clock* clock){
update_game(Memory* memory, Events* events, Clock* clock){
    assert(sizeof(PermanentMemory) < memory->permanent_size);
    assert(sizeof(TransientMemory) < memory->transient_size);
    pm = (PermanentMemory*)memory->permanent_base;
    tm = (TransientMemory*)memory->transient_base;


    //arena_free(render_buffer->render_command_arena);
    //push_clear_color(render_buffer->render_command_arena, BLACK);
    //Arena* render_command_arena = render_buffer->render_command_arena;
    if(!memory->initialized){
        pm->game_mode = GameMode_Game;
        init_camera();

        init_arena(&pm->arena, (u8*)memory->permanent_base + sizeof(PermanentMemory), memory->permanent_size - sizeof(PermanentMemory));
        init_arena(&tm->arena, (u8*)memory->transient_base + sizeof(TransientMemory), memory->transient_size - sizeof(TransientMemory));

        tm->render_command_arena = push_arena(&tm->arena, MB(16));
        tm->frame_arena = push_arena(&tm->arena, MB(100));

        // setup free entities array (max to 0)
        init_meshes(pm->meshes);
        entities_clear(pm);

        //pm->sprites_dir = str8_path_append(&pm->arena, path_data, str8_literal("sprites"));
        //pm->fonts_dir   = str8_path_append(&pm->arena, path_data, str8_literal("fonts"));
        //pm->saves_dir   = str8_path_append(&pm->arena, path_data, str8_literal("saves"));

        // basis test
        String8 tree_str   = str8_literal("tree00.bmp");
        String8 image_str  = str8_literal("image.bmp");
        String8 test_str   = str8_literal("test3.bmp");
        String8 circle_str = str8_literal("circle.bmp");
        String8 ship_str   = str8_literal("ship_simple.bmp");
        String8 bullet_str = str8_literal("bullet4.bmp");

        Bitmap image_image = load_bitmap(&tm->arena, path_sprites, image_str);
        Bitmap ship_image = load_bitmap(&tm->arena, path_sprites, ship_str);
        Bitmap tree_image = load_bitmap(&pm->arena, path_sprites, tree_str);
        Bitmap circle_image = load_bitmap(&tm->arena, path_sprites, circle_str);
        Bitmap bullet_image = load_bitmap(&tm->arena, path_sprites, bullet_str);
        //d3d_init_texture(ship_image);


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
        Entity* bullet = add_bullet(pm, origin, x_axis, y_axis, bullet_image, {0, 0, 0, 0});
        //add_rect(pm, rect_screen_to_pixel(make_rect(.1, .5f, .2, .6), resolution), MAGENTA);
        //add_rect(pm, rect_screen_to_pixel(make_rect(.3, .5f, .4, .6), resolution), MAGENTA, 4, GREEN);
        //add_rect(pm, rect_screen_to_pixel(make_rect(.5, .5f, .6, .6), resolution), MAGENTA, 0, BLUE);
        //add_rect(pm, rect_screen_to_pixel(make_rect(.7, .5f, .8, .6), resolution), MAGENTA, -20000, TEAL);

        first = add_cube(pm, ship_image, make_v3(10.0f, 0.0f, 60.0f), make_v3(0.0f, 0.0f, 0.0f), make_v3(0.2f, 0.2f, 0.2f), 120);
        second = add_player(pm, ship_image, make_v3(-10.0f, 0.0f, 60.0f), make_v3(0.0f, 0.0f, 0.0f), make_v3(0.2f, 0.2f, 0.2f), 121);
        //Inconsolata-Regular
        Bitmap inconsolate[128];

        //String8 incon = str8_literal("MatrixSans-Regular.ttf");
        //String8 incon = str8_literal("MatrixSans-Video.ttf");
        //String8 incon = str8_literal("Rock Jack Writing.ttf");
        //String8 incon = str8_literal("Inconsolata-Regular.ttf");
        String8 roboto = str8_literal("\\Roboto-Regular.ttf");
        String8 golos = str8_literal("\\GolosText-Regular.ttf");
        String8 arial = str8_literal("\\arial.ttf");
        String8 incon = str8_literal("\\consola.ttf");
        global_font.name = str8_literal("\\GolosText-Regular.ttf");
        global_font.size = 24;
        global_font.color = WHITE;
        bool succeed = load_font_ttf(&pm->arena, path_fonts, &global_font);
        assert(succeed);
        load_font_glyphs(&pm->arena, &global_font);

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

    if(pm->game_mode == GameMode_Game){
        if(controller.right.held){
            second->pos.x += 40 * (f32)clock->dt;
        }
        if(controller.left.held){
            second->pos.x -= 40 * (f32)clock->dt;
        }
        if(controller.up.held){
            second->pos.y += 40 * (f32)clock->dt;
        }
        if(controller.down.held){
            second->pos.y -= 40 * (f32)clock->dt;
        }
        if(controller.e.held){
            second->pos.z += 40 * (f32)clock->dt;
        }
        if(controller.q.held){
            second->pos.z -= 40 * (f32)clock->dt;
        }
    }
    if(has_flags(pm->game_mode, GameMode_Editor)){
        // up down
        if(controller.e.held){
            f32 dy = (f32)(camera.move_speed * clock->dt);
            camera.position.y += dy;
        }
        if(controller.q.held){
            f32 dy = (f32)(camera.move_speed * clock->dt);
            camera.position.y -= dy;
        }

        // wasd
        if(controller.up.held){
            v3 result = (camera.forward  * camera.move_speed * (f32)clock->dt);
            camera.position = camera.position + result;
        }
        if(controller.down.held){
            v3 result = (camera.forward  * camera.move_speed * (f32)clock->dt);
            camera.position = camera.position - result;
        }
        if(controller.left.held){
            v3 result = (normalized_v3(cross_product_v3(camera.forward, (v3){0, 1, 0})) * camera.move_speed * (f32)clock->dt);
            camera.position = camera.position + result;
        }
        if(controller.right.held){
            v3 result = (normalized_v3(cross_product_v3(camera.forward, (v3){0, 1, 0})) * camera.move_speed * (f32)clock->dt);
            camera.position = camera.position - result;
        }
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


    XMVECTOR camera_position = (XMVECTOR){camera.position.x, camera.position.y, camera.position.z};
    XMVECTOR camera_forward = (XMVECTOR){camera.forward.x, camera.forward.y, camera.forward.z};
    XMVECTOR camera_up = (XMVECTOR){camera.up.x, camera.up.y, camera.up.z};
    XMMATRIX view_matrix = XMMatrixLookAtLH(camera_position, camera_position + camera_forward, camera_up);
    XMMATRIX perspective_matrix = XMMatrixPerspectiveFovLH(PI_f32*0.25f, (f32)((f32)SCREEN_WIDTH/(f32)SCREEN_HEIGHT), 1.0f, 1000.0f);

    f32 aspect_ratio = (f32)SCREEN_HEIGHT / (f32)SCREEN_WIDTH;
    D3D11_MAPPED_SUBRESOURCE mapped_subresource;
    d3d_context->Map(constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);

    Constants* constants = (Constants*)mapped_subresource.pData;
    constants->transform =  view_matrix * perspective_matrix;
    d3d_context->Unmap(constant_buffer, 0);

    d3d_context->VSSetConstantBuffers(0, 1, &constant_buffer);


    //f32 background_color[4] = {0.2f, 0.29f, 0.29f, 1.0f};
    d3d_clear_color(BACKGROUND_COLOR);

    u32 c = array_count(pm->entities) - 1;
    for(u32 entity_index = (u32)pm->free_entities_at; entity_index < array_count(pm->entities); ++entity_index){
        Entity *e = pm->entities + pm->free_entities[entity_index];

        switch(e->type){
            case EntityType_Cube:{
                e->angle.z += (f32)clock->dt;
                e->angle.y += (f32)clock->dt;

                Mesh mesh = pm->meshes[EntityType_Cube];
                InstanceData* instance = instances + ((c - 1) - entity_index);
                f32 aspect_ratio = (f32)SCREEN_HEIGHT / (f32)SCREEN_WIDTH;
                instance->transform = XMMatrixTranspose(
                    XMMatrixRotationX(e->angle.x) *
                    XMMatrixRotationY(e->angle.y) *
                    XMMatrixRotationZ(e->angle.z) *
                    XMMatrixScaling(0.2f, 0.2f, 0.2f) *
                    XMMatrixTranslation(e->pos.x, e->pos.y, e->pos.z) *
                    XMMatrixPerspectiveLH(1.0f, aspect_ratio, 1.0f, 1000.0f)
                );
                //d3d_draw_cube_indexed(&mesh, e->texture, e->pos, e->angle, e->scale);
            } break;
            case EntityType_Player:{
                e->angle.z += (f32)clock->dt;
                e->angle.x += (f32)clock->dt;

                Mesh mesh = pm->meshes[EntityType_Cube];

                InstanceData* instance = instances + ((c - 1) - entity_index);
                f32 aspect_ratio = (f32)SCREEN_HEIGHT / (f32)SCREEN_WIDTH;
                instance->transform = XMMatrixTranspose(
                    XMMatrixRotationX(e->angle.x) *
                    XMMatrixRotationY(e->angle.y) *
                    XMMatrixRotationZ(e->angle.z) *
                    XMMatrixScaling(0.2f, 0.2f, 0.2f) *
                    XMMatrixTranslation(e->pos.x, e->pos.y, e->pos.z) *
                    XMMatrixPerspectiveLH(1.0f, aspect_ratio, 1.0f, 1000.0f)
                );
                //d3d_draw_cube_indexed(&mesh, e->texture, e->pos, e->angle, e->scale);
            } break;
        }
    }
    Mesh mesh = pm->meshes[EntityType_Cube];
    d3d_draw_cube_instanced(&mesh, &second->texture);


    //if(console_is_visible()){
    //    push_console(render_command_arena);
    //}
    String8 text = str8_literal("get! This is my program.\nIt renders fonts.\nHere is some dummy text 123.\nMore Dummy Text ONETWOTHREE\nEND OF DUMMY_TEXT_TEST.H OK");
    //String8 text   = str8_literal("g");
    String8 strings[] = {
        str8_literal("Console:"),
        str8_literal("  - Cursor - left/right movement. up/down through history. home/end."),
        str8_literal("  - Editing - start, middle, end of cursor position"),
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

    camera_update(make_v3(0.0, 0.0, 0.0));
    clear_controller_pressed(&controller);
    arena_free(&tm->arena);
}

#endif

