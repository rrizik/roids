#ifndef GAME_H
#define GAME_H

#include "math.h"
#include "rect.h"
#include "font.h"
#include "renderer.h"

#include "entity.h"
#include "console.h"

Font font_incon;

typedef struct PermanentMemory{
    Arena arena;
    String8 cwd; // CONSIDER: this might be something we want to be set on the platform side
    String8 data_dir; // CONSIDER: this might be something we want to be set on the platform side
    String8 fonts_dir; // CONSIDER: this might be something we want to be set on the platform side

    u32 generation[100];
    u32 free_entities[100];
    u32 free_entities_at;

    Entity entities[100];
    u32 entity_count;

    Entity* texture;
    Entity* circle;
    Entity* basis;
    Bitmap tree;

    //Console* console;
} PermanentMemory;

typedef struct TransientMemory{
    Arena arena;
    Arena *frame_arena;
    Arena *render_command_arena;
} TransientMemory;

static Entity*
entity_from_handle(PermanentMemory* pm, EntityHandle handle){
    Entity *result = 0;
    if(handle.index < ArrayCount(pm->entities)){
        Entity *e = pm->entities + handle.index;
        if(e->generation == handle.generation){
            result = e;
        }
    }
    return(result);
}

static EntityHandle
handle_from_entity(PermanentMemory* pm, Entity *e){
    Assert(e != 0);
    EntityHandle result = {0};
    if((e >= pm->entities) && (e < (pm->entities + ArrayCount(pm->entities)))){
        result.index = e->index;
        result.generation = e->generation;
    }
    return(result);
}

static void
remove_entity(PermanentMemory* pm, Entity* e){
    e->type = EntityType_None;
    pm->free_entities[++pm->free_entities_at] = e->index;
    pm->entity_count--;
    e->index = 0;
    e->generation = 0;
}

static Entity*
add_entity(PermanentMemory *pm, EntityType type){
    if(pm->free_entities_at >= 0){
        s32 free_entity_index = pm->free_entities[pm->free_entities_at--];
        Entity *e = pm->entities + free_entity_index;
        e->index = free_entity_index;
        pm->generation[e->index]++;
        pm->entity_count++;
        e->generation = pm->generation[e->index];
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
add_basis(PermanentMemory* pm, v2 origin, v2 x_axis, v2 y_axis, Bitmap texture, RGBA color){
    Entity* e = add_entity(pm, EntityType_Basis);
    e->origin = origin;
    e->x_axis = x_axis;
    e->y_axis = y_axis;
    e->color = color;
    e->texture = texture;
    return(e);
}

//static Entity*
//add_console(PermanentMemory* pm, Rect rect, RGBA color, s32 bsize = 0, RGBA bcolor = {0, 0, 0, 0}, bool bextrudes = false){
//    Entity* e = add_entity(pm, EntityType_Rect);
//    e->rect =  rect;
//    e->color = color;
//    e->border_size     = bsize;
//    e->border_color    = bcolor;
//    e->console_state   = CLOSED;
//    e->start_position  = e->rect.y0;
//    e->draw = false;
//    return(e);
//}

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

static Entity*
add_glyph(PermanentMemory* pm, v2 pos, Glyph glyph){
    Entity* e = add_entity(pm, EntityType_Glyph);
    e->rect = make_rect(pos.x, pos.y, 0, 0);
    e->glyph = glyph;
    return(e);
}

#define push_text_array(arena, pos, strings) _push_text_array(arena, pos, strings, array_count(strings))
static void _push_text_array(Arena* command_arena, v2 pos, String8 strings[], u32 count){
    u8* c;
    f32 scale = font_incon.scale;
    v2s32 unscaled_offset = {0, 0};
    for(u32 i=0; i < count; ++i){
       String8* string = strings + i;

        for(u32 i=0; i < string->size; ++i){
            c = string->str + i;
            if(*c != '\n'){
                Glyph glyph = font_incon.glyphs[*c];

                // get codepoint info
                s32 advance_width, lsb;
                stbtt_GetCodepointHMetrics(&font_incon.info, *c, &advance_width, &lsb);
                s32 x0, y0, x1, y1;
                stbtt_GetCodepointBitmapBox(&font_incon.info, *c, scale, scale, &x0,&y0,&x1,&y1);

                // setup rect
                Rect rect = {
                    pos.x + round_f32_s32((unscaled_offset.x + lsb) * scale),
                    pos.y - (round_f32_s32(unscaled_offset.y * scale) + (glyph.height + y0)),
                    0,
                    0
                };
                push_glyph(command_arena, rect, glyph);

                // advance on x
                unscaled_offset.x += advance_width;
                if(string->str[i + 1]){
                    s32 kern = stbtt_GetCodepointKernAdvance(&font_incon.info, *c, string->str[i+1]);
                    unscaled_offset.x += kern;
                }
            }
            else{
                // advance to next line
                unscaled_offset.y += font_incon.vertical_offset;
                unscaled_offset.x = 0;
            }
        }
        unscaled_offset.y += font_incon.vertical_offset;
        unscaled_offset.x = 0;
    }
}

static void
push_text(Arena* command_arena, v2 pos, String8 string, bool split_down = true){
    u8* c;
    s32 kern;
    f32 scale = font_incon.scale;
    v2s32 unscaled_offset = {0, 0};

    for(u32 i=0; i < string.size; ++i){
        c = string.str + i;
        if(*c != '\n'){
            Glyph glyph = font_incon.glyphs[*c];

            // get codepoint info
            s32 advance_width, lsb;
            stbtt_GetCodepointHMetrics(&font_incon.info, *c, &advance_width, &lsb);
            s32 x0, y0, x1, y1;
            stbtt_GetCodepointBitmapBox(&font_incon.info, *c, scale, scale, &x0,&y0,&x1,&y1);

            // setup rect to be pushed to command_arena
            Rect rect = {
                pos.x + (s32)round_f32_s32((unscaled_offset.x + lsb) * scale),
                pos.y - (s32)(round_f32_s32(unscaled_offset.y * scale) + (glyph.height + y0)),
                0, 0 // no x1, y1 needed for bitmap
            };
            push_glyph(command_arena, rect, glyph);

            // advance x + kern
            unscaled_offset.x += advance_width;
            if(string.str[i + 1]){
                kern = stbtt_GetCodepointKernAdvance(&font_incon.info, *c, string.str[i+1]);
                unscaled_offset.x += kern;
            }
        }
        else{
            // advance to next line
            if(split_down){
                unscaled_offset.y += font_incon.vertical_offset;
            }
            else{
                unscaled_offset.y -= font_incon.vertical_offset;
            }
            unscaled_offset.x = 0;
        }
    }
}

static void
draw_commands(RenderBuffer *render_buffer, Arena *commands){
    void* at = commands->base;
    void* end = (u8*)commands->base + commands->used;
    while(at != end){
        CommandHeader* base_command = (CommandHeader*)at;

        switch(base_command->type){
            case RenderCommand_ClearColor:{
                ClearColorCommand *command = (ClearColorCommand*)base_command;
                clear_color(render_buffer, command->ch.color);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Pixel:{
                PixelCommand *command = (PixelCommand*)base_command;
                draw_pixel(render_buffer, make_v2(command->ch.rect.x0, command->ch.rect.y0), command->ch.color);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Segment:{
                SegmentCommand *command = (SegmentCommand*)base_command;
                draw_segment(render_buffer, command->p0, command->p1, command->ch.color);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Ray:{
                RayCommand *command = (RayCommand*)base_command;
                draw_ray(render_buffer, command->ch.rect, command->ch.direction, command->ch.color);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Line:{
                LineCommand *command = (LineCommand*)base_command;
                draw_line(render_buffer, command->ch.rect, command->ch.direction, command->ch.color);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Rect:{
                RectCommand *command = (RectCommand*)base_command;
                draw_rect(render_buffer, command->ch.rect, command->ch.color);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Basis:{
                BasisCommand *command = (BasisCommand*)base_command;
                draw_rect_slow(render_buffer, command->ch.origin, command->ch.x_axis, command->ch.y_axis, &command->texture, command->ch.color);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Box:{
                BoxCommand *command = (BoxCommand*)base_command;
                draw_box(render_buffer, command->ch.rect, command->ch.color);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Quad:{
                QuadCommand *command = (QuadCommand*)base_command;
                draw_quad(render_buffer, command->p0, command->p1, command->p2, command->p3, command->ch.color, command->ch.fill);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Triangle:{
                TriangleCommand *command = (TriangleCommand*)base_command;
                draw_triangle(render_buffer, command->p0, command->p1, command->p2, base_command->color, base_command->fill);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Circle:{
                CircleCommand *command = (CircleCommand*)base_command;
                draw_circle(render_buffer, command->ch.rect.x0, command->ch.rect.y0, command->ch.rad, command->ch.color, command->ch.fill);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Bitmap:{
                BitmapCommand *command = (BitmapCommand*)base_command;
                draw_bitmap(render_buffer, command->ch.rect.min, &command->texture);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
            case RenderCommand_Glyph:{
                BitmapCommand *command = (BitmapCommand*)base_command;
                draw_bitmap(render_buffer, command->ch.rect.min, &command->texture);
                at = (u8*)commands->base + command->ch.arena_used;
            } break;
        }
    }
}

PermanentMemory* pm;
TransientMemory* tm;

u32 x_offset = 0;
f32 scale;
f32 angle;

static void
update_game(Memory* memory, RenderBuffer* render_buffer, Events* events, Controller* controller, Clock* clock){
    Assert(sizeof(PermanentMemory) < memory->permanent_size);
    Assert(sizeof(TransientMemory) < memory->transient_size);
    pm = (PermanentMemory*)memory->permanent_base;
    tm = (TransientMemory*)memory->transient_base;

    RGBA RED =     {1.0f, 0.0f, 0.0f,  1.0f};
    RGBA GREEN =   {0.0f, 1.0f, 0.0f,  1.0f};
    RGBA BLUE =    {0.0f, 0.0f, 1.0f,  1.0f};
    RGBA MAGENTA = {1.0f, 0.0f, 1.0f,  1.0f};
    RGBA TEAL =    {0.0f, 1.0f, 1.0f,  1.0f};
    RGBA PINK =    {0.92f, 0.62f, 0.96f, 1.0f};
    RGBA YELLOW =  {0.9f, 0.9f, 0.0f,  1.0f};
    RGBA ORANGE =  {1.0f, 0.5f, 0.15f,  1.0f};
    RGBA DGRAY =   {0.5f, 0.5f, 0.5f,  1.0f};
    RGBA LGRAY =   {0.8f, 0.8f, 0.8f,  1.0f};
    RGBA WHITE =   {1.0f, 1.0f, 1.0f,  1.0f};
    RGBA BLACK =   {0.0f, 0.0f, 0.0f,  1.0f};
    RGBA ARMY_GREEN =   {0.25f, 0.25, 0.23,  1.0f};


    if(!memory->initialized){
        angle = 0;

        init_arena(&pm->arena, (u8*)memory->permanent_base + sizeof(PermanentMemory), memory->permanent_size - sizeof(PermanentMemory));
        init_arena(&tm->arena, (u8*)memory->transient_base + sizeof(TransientMemory), memory->transient_size - sizeof(TransientMemory));
        pm->cwd = os_get_cwd(&pm->arena);
        pm->data_dir = str8_concatenate(&pm->arena, pm->cwd, str8_literal("\\data\\"));
        pm->fonts_dir = str8_concatenate(&pm->arena, pm->data_dir, str8_literal("\\fonts\\"));

        tm->render_command_arena = push_arena(&tm->arena, MB(16));
        tm->frame_arena = push_arena(&tm->arena, MB(100));

        // setup free entities array
        pm->free_entities_at = ArrayCount(pm->free_entities) - 1;
        for(s32 i = ArrayCount(pm->free_entities) - 1; i >= 0; --i){
            pm->free_entities[i] = ArrayCount(pm->free_entities) - 1 - i;
        }

        Entity *zero_entity = add_entity(pm, EntityType_None);
        //pm->console = add_console(pm, make_rect(0, 1, 1, 1), ARMY_GREEN);
        //add_rect(pm, screen_to_pixel(make_rect(.1, .1f, .2, .2), resolution), MAGENTA);
        //add_rect(pm, screen_to_pixel(make_rect(.3, .1f, .4, .2), resolution), MAGENTA, 4, GREEN);
        //add_rect(pm, screen_to_pixel(make_rect(.5, .1f, .6, .2), resolution), MAGENTA, 0, BLUE);
        //add_rect(pm, screen_to_pixel(make_rect(.7, .1f, .8, .2), resolution), MAGENTA, -20000, TEAL);


        // basis test
        String8 tree_str = str8_literal("tree00.bmp");
        pm->tree = load_bitmap(&tm->arena, pm->data_dir, tree_str);
        String8 circle_str = str8_literal("circle.bmp");
        Bitmap circle = load_bitmap(&tm->arena, pm->data_dir, circle_str);
        String8 image_str = str8_literal("image.bmp");
        Bitmap image = load_bitmap(&tm->arena, pm->data_dir, image_str);

		v2 origin = make_v2(resolution.x/2, resolution.y/2);
		v2 x_axis = 100.0f * make_v2(cos_f32(angle), sin_f32(angle));
		v2 y_axis = make_v2(-x_axis.y, x_axis.x);

        pm->basis = add_basis(pm, origin, x_axis, y_axis, pm->tree, RED);

        //add_bitmap(pm, make_v2(100, 100), pm->tree);
        //add_bitmap(pm, make_v2(400, 100), circle);

        //add_bitmap(pm, make_v2(0,   100), image);
        //add_bitmap(pm, make_v2(200, 100), image);
        //add_bitmap(pm, make_v2(400, 100), image);
        //add_bitmap(pm, make_v2(600, 100), image);
        //add_bitmap(pm, make_v2(800, 100), image);
        //add_bitmap(pm, make_v2(1000, 100), image);

        //add_bitmap(pm, make_v2(0,   300), image);
        //add_bitmap(pm, make_v2(200, 300), image);
        //add_bitmap(pm, make_v2(400, 300), image);
        //add_bitmap(pm, make_v2(600, 300), image);
        //add_bitmap(pm, make_v2(800, 300), image);
        //add_bitmap(pm, make_v2(1000, 300), image);

        //add_bitmap(pm, make_v2(0,   500), image);
        //add_bitmap(pm, make_v2(200, 500), image);
        //add_bitmap(pm, make_v2(400, 500), image);
        //add_bitmap(pm, make_v2(600, 500), image);
        //add_bitmap(pm, make_v2(800, 500), image);
        //add_bitmap(pm, make_v2(1000, 500), image);

#if 0

        String8 texture = str8_literal("texture.bmp");
        String8 circle = str8_literal("circle.bmp");
        Bitmap bmp_texture = load_bitmap(&tm->arena, pm->data_dir, texture);
        Bitmap bmp_circle = load_bitmap(&tm->arena, pm->data_dir, circle);
        pm->texture = add_bitmap(pm, make_v2(300, 300), bmp_texture);
        pm->circle = add_bitmap(pm, make_v2(500, 500), bmp_circle);
#endif
        //Inconsolata-Regular
        Bitmap inconsolate[128];

        //String8 incon = str8_literal("MatrixSans-Regular.ttf");
        //String8 incon = str8_literal("MatrixSans-Video.ttf");
        //String8 incon = str8_literal("Rock Jack Writing.ttf");
        //String8 incon = str8_literal("Inconsolata-Regular.ttf");
        String8 incon = str8_literal("GolosText-Regular.ttf");
        //String8 incon = str8_literal("arial.ttf");
        bool succeed = load_font_ttf(&pm->arena, pm->fonts_dir, incon, &font_incon);
        assert(succeed);
        load_font_glyphs(&pm->arena, 50, ORANGE, &font_incon);
        init_console();

        memory->initialized = true;
    }
    arena_free(render_buffer->render_command_arena);
    push_clear_color(render_buffer->render_command_arena, BLUE);
    angle += clock->dt;
    //print("angle: %f\n", angle);



    // NOTE: Process events.
    while(!events_empty(events)){
        Event event = event_get(events);

        if(event.type == TEXT_INPUT){
            Glyph glyph = font_incon.glyphs[event.keycode];
            //add_glyph(pm, make_v2(10 + x_offset, 10), glyph);
            x_offset += glyph.width;
            print("text_input: %i - %c\n", event.keycode, event.keycode);
            print("-----------------------------\n");
        }
        if(event.type == KEYBOARD){
            if(event.key_pressed){
                //print("key_code: %llu\n", event.keycode);
                if(event.keycode == ESCAPE){
                    print("quiting\n");
                    should_quit = true;
                }

                if(event.keycode == TILDE && !event.repeat){
                    console_mark();

                    if(event.shift_pressed){
                        if(console_state == OPEN_BIG){
                            console_set_state(CLOSED);
                        }
                        else{ console_set_state(OPEN_BIG); }
                    }
                    else{
                        if(console_state == OPEN){
                            console_set_state(CLOSED);
                        }
                        else{ console_set_state(OPEN); }

                    }
                }

            }
            else{
            }
        }
    }

    update_console();

    Arena* render_command_arena = render_buffer->render_command_arena;
    for(u32 entity_index = pm->free_entities_at; entity_index < ArrayCount(pm->entities); ++entity_index){
        Entity *e = pm->entities + pm->free_entities[entity_index];

        switch(e->type){
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
            case EntityType_Basis:{
				v2 dim = {5, 5};
				v2 min = e->origin;
                RGBA color = {1, 1, 0, 1};
                f32 disp = 50.0f * cos_f32(angle);
                e->origin = make_v2(resolution.x/2, resolution.y/2);
#if 0
                e->x_axis = 400 * make_v2(cos_f32(angle/10), sin_f32(angle/10));
                e->y_axis = perp(e->x_axis);
                //e->x_axis = (50.0f + 50.0f * cos_f32(angle*2)) * make_v2(cos_f32(angle*2), sin_f32(angle*2));
                //e->y_axis = (50.0f + 50.0f * cos_f32(angle*2)) * make_v2(cos_f32((angle*2) + 1.0f), sin_f32((angle*2) + 1.0f));
                //e->y_axis = make_v2(-e->x_axis.y, e->x_axis.x);
#else
                e->x_axis = {400, 0};
                e->y_axis = {0, 400};
#endif

                push_basis(render_command_arena, make_v2(disp, 0) + (e->origin - (0.5f * e->x_axis) - (0.5f * e->y_axis)), e->x_axis, e->y_axis, e->texture, RED);


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
            case EntityType_Glyph:{
                push_glyph(render_command_arena, e->rect, e->glyph);
            }break;
            case EntityType_None:{
            }break;
            case EntityType_Object:{
            }break;
            invalid_default_case;
        }
    }

    if(console_is_open()){
        //push_console(render_command_arena);
    }
    String8 one = str8_literal("get! This is my program.\nIt renders fonts.\nHere is some dummy text 123.\nMore Dummy Text ONETWOTHREE\nEND OF DUMMY_TEXT_TEST.H OK");
    //String8 one   = str8_literal("g");
    String8 strings[] = {
        str8_literal("get! This is my program."),
        str8_literal("It renders fonts."),
        str8_literal("Here is some dummy text 123."),
        str8_literal("More Dummy Text ONETWOTHREE"),
        str8_literal("END OF DUMMY_TEXT_TEST.H OK"),
    };
    //push_text_array(render_command_arena, make_v2(10, resolution.h - 50), strings);
    //push_text(render_command_arena, make_v2(100, 600), one);

    //push_text(render_command_arena, make_v2(0, resolution.h - 100), two);
    //push_text(render_command_arena, make_v2(0, resolution.h - 150), three);
    //push_text(render_command_arena, make_v2(0, resolution.h - 200), four);
    //push_text(render_command_arena, make_v2(0, resolution.h - 250), five);
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
    draw_bitmap(render_buffer, make_v2(100, 100), &pm->tree);
}

#endif
