#ifndef GAME_H
#define GAME_H

#include "math.h"
#include "rect.h"
#include "bitmap.h"
#include "font.h"
#include "renderer.h"

#include "entity.h"

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

#define ENTITIES_MAX 100
typedef struct PermanentMemory{
    Arena arena;
    String8 cwd; // CONSIDER: this might be something we want to be set on the platform side
    String8 data_dir; // CONSIDER: this might be something we want to be set on the platform side
    String8 fonts_dir; // CONSIDER: this might be something we want to be set on the platform side
    String8 sprites_dir; // CONSIDER: this might be something we want to be set on the platform side
    String8 saves_dir; // CONSIDER: this might be something we want to be set on the platform side

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

} PermanentMemory;
global PermanentMemory* pm;

typedef struct TransientMemory{
    Arena arena;
    Arena *frame_arena;
    Arena *render_command_arena;
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
add_basis(PermanentMemory* pm, v2 origin, v2 x_axis, v2 y_axis, Bitmap texture, RGBA color = {0, 0, 0, 1}){
    Entity* e = add_entity(pm, EntityType_Basis);
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
    e->scale = 50;
    pm->ship_loaded = true; // TODO: get rid of
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
    // TODO: CLEANUP: INCOMPLETE: YUCK: This is garbage. Make String8 better so I can build strings and paths better
    os_file_create(pm->saves_dir, filename, 1);
    u32 offset = 0;
    for(u32 i=0; i < array_count(pm->entities); ++i){
        Entity* e = pm->entities + i;
        if(e->type != EntityType_None){
            size_t size = sizeof(*e);
            FileData data = {
                .base = e,
                .size = size,
            };
            os_file_write(data, pm->saves_dir, filename, offset);
            offset += size;
        }
    }
}

static void
deserialize_data(PermanentMemory* pm, String8 filename){
    // TODO: CLEANUP: INCOMPLETE: YUCK: This is garbage. Make String8 better so I can build strings and paths better
    FileData data;
    bool succeed = os_file_read(&data, &pm->arena, pm->saves_dir, filename);
    if(succeed){
        entities_clear(pm);
        u32 offset = 0;
        while(offset < data.size){
            Entity* e = (Entity*)((u8*)data.base + offset);
            switch(e->type){
                case EntityType_Ship:{
                    Entity* ship = add_entity(pm, EntityType_Ship);
                    *ship = *e;

                    String8 ship_str = str8_literal("\\ship_simple.bmp");
                    Bitmap ship_image = load_bitmap(&tm->arena, pm->sprites_dir, ship_str);
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
}

#include "console.h"

static bool
handle_global_event(Event event){
    if(event.type == KEYBOARD){
        if(event.key_pressed){
            if(event.keycode == ESCAPE){
                //print("quiting\n");
                //should_quit = true;
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
        }
    }
    return(false);
}


static bool
handle_controller_events(Event event){
    if(event.type == KEYBOARD){
        if(event.key_pressed){
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
            if(event.keycode == O_UPPER){
                if(!event.repeat){
                    controller.ser.pressed = true;
                }
                controller.ser.held = true;
                return(true);
            }
            if(event.keycode == L_UPPER){
                if(!event.repeat){
                    controller.deser.pressed = true;
                }
                controller.deser.held = true;
                return(true);
            }
        }
        else{
            if(event.keycode == A_UPPER){
                controller.left.held = false;
                return(true);
            }
            if(event.keycode == D_UPPER){
                controller.right.held = false;
                return(true);
            }
            if(event.keycode == W_UPPER){
                controller.up.held = false;
                return(true);
            }
            if(event.keycode == S_UPPER){
                controller.down.held = false;
                return(true);
            }
            if(event.keycode == O_UPPER){
                controller.ser.held = false;
                return(true);
            }
            if(event.keycode == L_UPPER){
                controller.deser.held = false;
                return(true);
            }
        }
    }
    return(false);
}

static void
update_game(Memory* memory, RenderBuffer* render_buffer, Events* events, Clock* clock){
    assert(sizeof(PermanentMemory) < memory->permanent_size);
    assert(sizeof(TransientMemory) < memory->transient_size);
    pm = (PermanentMemory*)memory->permanent_base;
    tm = (TransientMemory*)memory->transient_base;


    arena_free(render_buffer->render_command_arena);
    push_clear_color(render_buffer->render_command_arena, BLACK);
    Arena* render_command_arena = render_buffer->render_command_arena;
    if(!memory->initialized){
        Button a = controller.up;

        init_arena(&pm->arena, (u8*)memory->permanent_base + sizeof(PermanentMemory), memory->permanent_size - sizeof(PermanentMemory));
        init_arena(&tm->arena, (u8*)memory->transient_base + sizeof(TransientMemory), memory->transient_size - sizeof(TransientMemory));

        tm->render_command_arena = push_arena(&tm->arena, MB(16));
        tm->frame_arena = push_arena(&tm->arena, MB(100));

        // setup free entities array (max to 0)
        entities_clear(pm);

        pm->cwd = os_get_cwd(&pm->arena);
        pm->data_dir    = str8_path_append(&pm->arena, pm->cwd,      str8_literal("data"));
        pm->sprites_dir = str8_path_append(&pm->arena, pm->data_dir, str8_literal("sprites"));
        pm->fonts_dir   = str8_path_append(&pm->arena, pm->data_dir, str8_literal("fonts"));
        pm->saves_dir   = str8_path_append(&pm->arena, pm->data_dir, str8_literal("saves"));

        // basis test
        String8 tree_str   = str8_literal("tree00.bmp");
        String8 image_str  = str8_literal("image.bmp");
        String8 test_str   = str8_literal("test3.bmp");
        String8 circle_str = str8_literal("circle.bmp");
        String8 ship_str   = str8_literal("ship_simple.bmp");

        Bitmap image_image = load_bitmap(&tm->arena, pm->sprites_dir, image_str);
        Bitmap ship_image = load_bitmap(&tm->arena, pm->sprites_dir, ship_str);
        Bitmap tree_image = load_bitmap(&pm->arena, pm->sprites_dir, tree_str);
        Bitmap circle_image = load_bitmap(&tm->arena, pm->sprites_dir, circle_str);

        //Bitmap ship_image = load_bitmap(&pm->arena, pm->sprites_dir, ship_str);
        //Bitmap aa = stb_load_image(pm->sprites_dir, circle_str);
        //Bitmap ship_image = stb_load_image(pm->sprites_dir, ship_str);
        //Bitmap test_image0 = stb_load_image(pm->sprites_dir, test_str);
        //Bitmap test_image1 = stb_load_image(pm->sprites_dir, test_str);
        //Bitmap test_image1 = load_bitmap(&pm->arena, pm->sprites_dir, test_str);

		v2 origin = make_v2((f32)resolution.x/2, (f32)resolution.y/2);
        v2 direction = make_v2(0, 1);
		v2 x_axis = 100.0f * make_v2(cos_f32(dir_to_rad(direction)), sin_f32(dir_to_rad(direction)));
		v2 y_axis = make_v2(-x_axis.y, x_axis.x);
        //pm->ship = add_basis(pm, origin, x_axis, y_axis, ship_image, {0, 0, 0, 0});
        //add_basis(pm, origin, x_axis, y_axis, tree_image, {0, 0, 0, 0});
        //add_basis(pm, origin, x_axis, y_axis, circle_image, {0, 0, 0, 0});
        //add_basis(pm, origin, x_axis, y_axis, test_image0, {0, 0, 0, 0});
        //pm->ship = add_ship(pm, make_v2(100, 100), x_axis, y_axis, ship_image, {0, 0, 0, 0});
        //add_rect(pm, rect_screen_to_pixel(make_rect(.1, .5f, .2, .6), resolution), MAGENTA);
        //add_rect(pm, rect_screen_to_pixel(make_rect(.3, .5f, .4, .6), resolution), MAGENTA, 4, GREEN);
        //add_rect(pm, rect_screen_to_pixel(make_rect(.5, .5f, .6, .6), resolution), MAGENTA, 0, BLUE);
        //add_rect(pm, rect_screen_to_pixel(make_rect(.7, .5f, .8, .6), resolution), MAGENTA, -20000, TEAL);

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
        bool succeed = load_font_ttf(&pm->arena, pm->fonts_dir, &global_font);
        assert(succeed);
        load_font_glyphs(&pm->arena, &global_font);

        init_console(pm);
        init_commands();

        memory->initialized = true;
    }


    // NOTE: process events.
    while(!events_empty(events)){
        Event event = events_get(events);

        bool handled;
        handled = handle_global_event(event);

        if(console_is_open()){
            handled = handle_console_event(event);
        }
        else{
            handled = handle_controller_events(event);

        }
    }

    // serialize deserialize
    //if(controller.ser.pressed){
    //    serialize_data(pm, str8_literal("\\save1.r"));
    //}
    //if(controller.deser.pressed){
    //    deserialize_data(pm, str8_literal("\\save1.r"));
    //}

    if(pm->ship_loaded){
        Entity* ship = pm->ship;
        // rotate ship
        if(controller.right.held){
            ship->rad -= 2 * (f32)clock->dt;
        }
        if(controller.left.held){
            ship->rad += 2 * (f32)clock->dt;
        }

        // increase ship velocity
        if(controller.up.held){
            ship->velocity += (f32)clock->dt;
        }
        if(controller.down.held){
            ship->velocity -= (f32)clock->dt;
        }
        clamp_f32(0, 1, &ship->velocity);

        // move ship
        ship->direction = rad_to_dir(ship->rad);
        ship->origin.x += (ship->direction.x * ship->velocity * ship->speed) * (f32)clock->dt;
        ship->origin.y += (ship->direction.y * ship->velocity * ship->speed) * (f32)clock->dt;
        //print("x: %f - y: %f - v: %f - a: %f\n", ship->direction.x, ship->direction.y, ship->velocity, ship->rad);
    }

    update_console();
    for(u32 entity_index = (u32)pm->free_entities_at; entity_index < array_count(pm->entities); ++entity_index){
        Entity *e = pm->entities + pm->free_entities[entity_index];

        switch(e->type){
            case EntityType_Glyph:{
            }break;
            case EntityType_Pixel:{
                push_pixel(render_command_arena, e->rect, e->color);
            }break;
            case EntityType_Segment:{
                push_segment(render_command_arena, e->p0, e->p1, e->color);
            }break;
            case EntityType_Line:{
                push_line(render_command_arena, e->rect, e->direction, e->color);
            }break;
            case EntityType_Ray:{
                push_ray(render_command_arena, e->rect, e->direction, e->color);
            }break;
            case EntityType_Rect:{
                Rect border;
                Rect rect = e->rect;
                if(e->border_size > 0){
                    border = rect_calc_border(e->rect, e->border_size);
                }
                if(e->border_size < 0){
                    border = e->rect;
                    rect = rect_calc_border(e->rect, e->border_size);
                }
                push_rect(render_command_arena, border, e->border_color);
                push_rect(render_command_arena, rect, e->color);
            }break;
            case EntityType_Ship:{
                f32 deg = rad_to_deg(e->rad);
                deg -= 90;
                f32 rad = deg_to_rad(deg);
                e->x_axis = e->scale * make_v2(cos_f32(rad), sin_f32(rad));
                e->y_axis = perp(e->x_axis);
                v2 center_org = e->origin - 0.5*e->x_axis - 0.5*e->y_axis;

                push_basis(render_command_arena, center_org, e->x_axis, e->y_axis, e->texture);
            }break;
            case EntityType_Basis:{
				v2 dim = {5, 5};
				v2 min = e->origin;
                //RGBA color = {1, 1, 0, 1};
                //f32 disp = 50.0f * cos_f32(angle);
                //e->origin = make_v2((f32)resolution.x/2, (f32)resolution.y/2);
                f32 deg = rad_to_deg(e->rad);
                deg -= 90;
                f32 rad = deg_to_rad(deg);
                e->x_axis = e->scale * make_v2(cos_f32(rad), sin_f32(rad));
                e->y_axis = perp(e->x_axis);
                //
                //e->x_axis = (50.0f + 50.0f * cos_f32(angle*2)) * make_v2(cos_f32(angle*2), sin_f32(angle*2));
                //e->y_axis = (50.0f + 50.0f * cos_f32(angle*2)) * make_v2(cos_f32((angle*2) + 1.0f), sin_f32((angle*2) + 1.0f));
                //
                //e->y_axis = make_v2(-e->x_axis.y, e->x_axis.x);
                //e->x_axis = {300, 0};
                //e->y_axis = make_v2(-e->x_axis.y, e->x_axis.x);
                //e->y_axis = {0, 400};

                push_basis(render_command_arena, e->origin - 0.5*e->x_axis - 0.5*e->y_axis, e->x_axis, e->y_axis, e->texture);

                //push_rect(render_command_arena, make_rect(min - dim, min + dim), color);

				min = e->origin + e->x_axis;
                //push_rect(render_command_arena, make_rect(min - dim, min + dim), color);

				min = e->origin + e->y_axis;
                //push_rect(render_command_arena, make_rect(min - dim, min + dim), color);

                v2 max = e->origin + e->x_axis + e->y_axis;
                //push_rect(render_command_arena, make_rect(max - dim, max + dim), color);

            }break;
            case EntityType_Box:{
                push_box(render_command_arena, e->rect, e->color);
            }break;
            case EntityType_Quad:{
                push_quad(render_command_arena, e->p0, e->p1, e->p2, e->p3, e->color, e->fill);
            }break;
            case EntityType_Triangle:{
                push_triangle(render_command_arena, e->p0, e->p1, e->p2, e->color, e->fill);
            }break;
            case EntityType_Circle:{
                push_circle(render_command_arena, e->rect, e->rad, e->color, e->fill);
            }break;
            case EntityType_Bitmap:{
                push_bitmap(render_command_arena, e->rect, e->texture);
            }break;
            case EntityType_None:{
            }break;
            case EntityType_Object:{
            }break;
            invalid_default_case;
        }
    }

    if(console_is_visible()){
        push_console(render_command_arena);
    }
    String8 text = str8_literal("get! This is my program.\nIt renders fonts.\nHere is some dummy text 123.\nMore Dummy Text ONETWOTHREE\nEND OF DUMMY_TEXT_TEST.H OK");
    //String8 text   = str8_literal("g");
    String8 strings[] = {
        str8_literal("Console:"),
        str8_literal("  - Cursor - left/right movement. up/down through history. home/end."),
        str8_literal("  - Editing - start, middle, end of cursor position"),
        str8_literal("  - Commands - console commands like (help, exit, add, save, load)"),
        str8_literal("                           - save/load will serialize/deserialize entity data"),
    };
    push_text_array(render_command_arena, make_v2(((f32)resolution.x/2.0f) - 50, ((f32)resolution.h/2.0f) -250), &global_font, strings, array_count(strings));
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
