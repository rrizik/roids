#ifndef RENDERER_H
#define RENDERER_H

// SIMD
#include <emmintrin.h>

// IMPORTANT
// UNTESTED We changed our Rect type to screenspace, and changed all functions. Some tested, some not.
// CLEANUP
#pragma pack(push, 1)
typedef struct BitmapHeader {
    u16 file_type;
	u32 file_size;
	u16 reserved1;
	u16 reserved2;
	u32 bitmap_offset;
	u32 header_size;
	s32 width;
	s32 height;
	u16 planes;
	u16 bits_per_pixel;
    u32 Compression;
	u32 SizeOfBitmap;
	s32 HorzResolution;
	s32 VertResolution;
	u32 ColorsUsed;
	u32 ColorsImportant;
//    u32 RedMask;
//    u32 GreenMask;
//    u32 BlueMask;
} BitmapHeader;
#pragma pack(pop)

typedef struct Bitmap{
    u8*  pixels;
	u32  width;
	u32  height;
} Bitmap;

// CONSIDER: do we need to pass in arena here? and if we do, why don't we use it to allocate an arena type instead of using it for os_file_read()
static Bitmap
load_bitmap(Arena *arena, String8 dir, String8 file_name){
    Bitmap result = {0};

    FileData bitmap_file = os_file_read(arena, dir, file_name);
    if(bitmap_file.size > 0){
        BitmapHeader *header = (BitmapHeader *)bitmap_file.base;
        result.pixels = (u8 *)bitmap_file.base + header->bitmap_offset;
        result.width = header->width;
        result.height = header->height;
        // IMPORTANT: depending on compression type you might have to re order the bytes to make it AA RR GG BB
        // as well as consider the masks for each color included in the header
    }
    return(result);
}

// TODO: make this simpler {}
static RGBA
u32_to_rgba(u32 value){
	f32 alpha = ((f32)((value >> 24) & 0xFF) / 255.0f);
	f32 red =   ((f32)((value >> 16) & 0xFF) / 255.0f);
	f32 green = ((f32)((value >> 8) & 0xFF) / 255.0f);
	f32 blue =  ((f32)((value >> 0) & 0xFF) / 255.0f);
    RGBA result = {red, green, blue, alpha};
    return result;
}

typedef enum RenderCommandType{
    RenderCommand_ClearColor,
    RenderCommand_Pixel,
    RenderCommand_Segment,
    RenderCommand_Line,
    RenderCommand_Ray,
    RenderCommand_Rect,
    RenderCommand_Box,
    RenderCommand_Quad,
    RenderCommand_Triangle,
    RenderCommand_Circle,
    RenderCommand_Bitmap,
    RenderCommand_Glyph,
} RenderCommandType;

typedef struct CommandHeader{
    RenderCommandType type;
    Rect rect;
    v2  direction;
    s32 border_size;

    u8 rad;

    RGBA color;
    RGBA border_color;

    v4   clip_region;
    bool fill;

    size_t arena_used;
} CommandHeader;

typedef struct ClearColorCommand{
    CommandHeader ch;
} ClearColorCommand;

typedef struct PixelCommand{
    CommandHeader ch;
} PixelCommand;

typedef struct SegmentCommand{
    CommandHeader ch;
    v2 p0;
    v2 p1;
} SegmentCommand;

typedef struct RayCommand{
    CommandHeader ch;
} RayCommand;

typedef struct LineCommand{
    CommandHeader ch;
} LineCommand;

typedef struct RectCommand{
    CommandHeader ch;
} RectCommand;

typedef struct BoxCommand{
    CommandHeader ch;
} BoxCommand;

typedef struct QuadCommand{
    CommandHeader ch;
    v2 p0;
    v2 p1;
    v2 p2;
    v2 p3;
} QuadCommand;

typedef struct TriangleCommand{
    CommandHeader ch;
    v2 p0;
    v2 p1;
    v2 p2;
} TriangleCommand;

typedef struct CircleCommand{
    CommandHeader ch;
} CircleCommand;

typedef struct BitmapCommand{
    CommandHeader ch;
    Bitmap image;
} BitmapCommand;

typedef struct GlyphCommand{
    CommandHeader ch;
    Glyph image;
} GlyphCommand;

static void
push_clear_color(Arena *arena, RGBA color){
    ClearColorCommand* command = push_struct(arena, ClearColorCommand);
    command->ch.type = RenderCommand_ClearColor,
    command->ch.arena_used = arena->used;
    command->ch.color = color;
}

static void
push_clear_color_region(Arena *arena, Rect rect, RGBA color){
    ClearColorCommand* command = push_struct(arena, ClearColorCommand);
    command->ch.type = RenderCommand_ClearColor,
    command->ch.color = color;
    command->ch.rect = rect;
    command->ch.arena_used = arena->used;
}


static void
push_pixel(Arena *arena, Rect rect, RGBA color){
    PixelCommand* command = push_struct(arena, PixelCommand);
    command->ch.type = RenderCommand_Pixel;
    command->ch.arena_used = arena->used;
    command->ch.rect = rect;
    command->ch.color = color;
}

static void
push_segment(Arena *arena, v2 p0, v2 p1, RGBA color){
    SegmentCommand* command = push_struct(arena, SegmentCommand);
    command->ch.type = RenderCommand_Segment;
    command->ch.arena_used = arena->used;
    command->ch.color = color;
    command->p0 = p0;
    command->p1 = p1;
}

static void
push_ray(Arena *arena, Rect rect, v2 direction, RGBA color){
    RayCommand* command = push_struct(arena, RayCommand);
    command->ch.type = RenderCommand_Ray;
    command->ch.arena_used = arena->used;
    command->ch.rect = rect;
    command->ch.direction = direction;
    command->ch.color = color;
}

static void
push_line(Arena *arena, Rect rect, v2 direction, RGBA color){
    LineCommand* command = push_struct(arena, LineCommand);
    command->ch.type = RenderCommand_Line;
    command->ch.arena_used = arena->used;
    command->ch.rect = rect;
    command->ch.color = color;
    command->ch.direction = direction;
}

static void
push_rect(Arena *arena, Rect rect, RGBA color, s32 border_size = 0, RGBA border_color = {0, 0, 0, 0}){
    RectCommand* command = push_struct(arena, RectCommand);
    command->ch.type = RenderCommand_Rect;
    command->ch.arena_used = arena->used;
    command->ch.rect = rect;
    command->ch.color = color;
    command->ch.border_size = border_size;
    command->ch.border_color = border_color;
}

static void
push_box(Arena *arena, Rect rect, RGBA color){
    BoxCommand* command = push_struct(arena, BoxCommand);
    command->ch.type = RenderCommand_Box;
    command->ch.arena_used = arena->used;
    command->ch.color = color;
    command->ch.rect = rect;
}

static void
push_quad(Arena *arena, v2 p0, v2 p1, v2 p2, v2 p3, RGBA color, bool fill){
    QuadCommand* command = push_struct(arena, QuadCommand);
    command->ch.type = RenderCommand_Quad;
    command->ch.arena_used = arena->used;
    command->ch.color = color;
    command->ch.fill = fill;
    command->p0 = p0;
    command->p1 = p1;
    command->p2 = p2;
    command->p3 = p3;
}

static void
push_triangle(Arena *arena, v2 p0, v2 p1, v2 p2, RGBA color, bool fill){
    TriangleCommand* command = push_struct(arena, TriangleCommand);
    command->ch.type = RenderCommand_Triangle;
    command->ch.arena_used = arena->used;
    command->ch.color = color;
    command->ch.fill = fill;
    command->p0 = p0;
    command->p1 = p1;
    command->p2 = p2;
}

static void
push_circle(Arena *arena, Rect rect, u8 rad, RGBA color, bool fill){
    CircleCommand* command = push_struct(arena, CircleCommand);
    command->ch.type = RenderCommand_Circle;
    command->ch.arena_used = arena->used;
    command->ch.rect = rect;
    command->ch.color = color;
    command->ch.fill = fill;
    command->ch.rad = rad;
}

static void
push_bitmap(Arena *arena, Rect rect, Bitmap image){
    BitmapCommand* command = push_struct(arena, BitmapCommand);
    command->ch.type = RenderCommand_Bitmap;
    command->ch.arena_used = arena->used;
    command->ch.rect = rect;
    command->image = image;
}

static void
push_glyph(Arena *arena, Rect rect, Glyph image){
    GlyphCommand* command = push_struct(arena, GlyphCommand);
    command->ch.type = RenderCommand_Glyph;
    command->ch.arena_used = arena->used;
    command->ch.rect = rect;
    command->image = image;
}

static void
draw_pixel(RenderBuffer *render_buffer, v2 position, RGBA color){
    v2s32 pos = round_v2_v2s32(position);

    if(pos.x >= 0 && pos.x < render_buffer->width && pos.y >= 0 && pos.y < render_buffer->height){
        u8 *row = (u8 *)render_buffer->base +
                   ((render_buffer->height - pos.y - 1) * render_buffer->stride) +
                   (pos.x * render_buffer->bytes_per_pixel);
        u32 *pixel = (u32 *)row;

        if(color.a == 1){
            u32 new_color = (round_f32_s32(color.a * 255.0f) << 24 | round_f32_s32(color.r * 255.0f) << 16 | round_f32_s32(color.g * 255.0f) << 8 | round_f32_s32(color.b * 255.0f) << 0);
            *pixel = new_color;
        }
        else if(color.a > 0){
            f32 current_a = ((f32)((*pixel >> 24) & 0xFF) / 255.0f);
            f32 current_r = (f32)((*pixel >> 16) & 0xFF);
            f32 current_g = (f32)((*pixel >> 8) & 0xFF);
            f32 current_b = (f32)((*pixel >> 0) & 0xFF);

            color.r *= 255.0f;
            color.g *= 255.0f;
            color.b *= 255.0f;

            f32 new_r = (1 - color.a) * current_r + (color.a * color.r);
            f32 new_g = (1 - color.a) * current_g + (color.a * color.g);
            f32 new_b = (1 - color.a) * current_b + (color.a * color.b);

            u32 new_color = (round_f32_s32(color.a * 255.0f) << 24 | round_f32_s32(new_r) << 16 | round_f32_s32(new_g) << 8 | round_f32_s32(new_b) << 0);
            *pixel = new_color;
        }
    }
}

// UNTESTED: untested with rect screenspace change
static void
draw_line(RenderBuffer *render_buffer, Rect rect, v2 direction, RGBA color){
    //RectPixelSpace rect_ps = screen_to_pixel(rect, resolution);

    v2 point1 = round_v2(rect.min);
    v2 point2 = point1;
    v2 non_normalized_direction = (v2){point1.x + direction.x, point1.y + direction.y};
    direction = round_v2(non_normalized_direction);

    f32 distance_x =  abs_f32(direction.x - point1.x);
    f32 distance_y = -abs_f32(direction.y - point1.y);
    f32 d1_step_x = point1.x < direction.x ? 1.0f : -1.0f;
    f32 d1_step_y = point1.y < direction.y ? 1.0f : -1.0f;
    f32 d2_step_x = -(d1_step_x);
    f32 d2_step_y = -(d1_step_y);

    f32 error = distance_x + distance_y;

    for(;;){
        draw_pixel(render_buffer, point1, color); // NOTE: before break, so you can draw a single point draw_segment (a pixel)
        if(point1.x < 0 || point1.x > render_buffer->width || point1.y < 0 || point1.y > render_buffer->height)break;

        f32 error2 = 2 * error;
        if (error2 >= distance_y){
            error += distance_y;
            point1.x += d1_step_x;
        }
        if (error2 <= distance_x){
            error += distance_x;
            point1.y += d1_step_y;
        }
    }
    for(;;){
        draw_pixel(render_buffer, point2, color);
        if(point2.x < 0 || point2.x > render_buffer->width || point2.y < 0 || point2.y > render_buffer->height)break;

        f32 error2 = 2 * error;
        if (error2 >= distance_y){
            error += distance_y;
            point2.x += d2_step_x;
        }
        if (error2 <= distance_x){
            error += distance_x;
            point2.y += d2_step_y;
        }
    }
}

// UNTESTED: untested with rect screenspace change
static void
draw_ray(RenderBuffer *render_buffer, Rect rect, v2 direction, RGBA color){
    //RectPixelSpace rect_ps = screen_to_pixel(rect, resolution);
    v2 pos = round_v2(rect.min);

    v2 non_normalized_direction = round_v2((v2){(direction.x * 100000), (direction.y * 100000)});
    v2 new_direction = (v2){pos.x + non_normalized_direction.x, pos.y + non_normalized_direction.y};

    f32 distance_x =  abs_f32(new_direction.x - pos.x);
    f32 distance_y = -abs_f32(new_direction.y - pos.y);
    f32 step_x = pos.x < new_direction.x ? 1.0f : -1.0f;
    f32 step_y = pos.y < new_direction.y ? 1.0f : -1.0f;

    f32 error = distance_x + distance_y;

    for(;;){
        draw_pixel(render_buffer, make_v2(pos.x, pos.y), color); // NOTE: before break, so you can draw a single position draw_segment (a pixel)
        if(pos.x < 0 || pos.x > render_buffer->width || pos.y < 0 || pos.y > render_buffer->height)break;

        f32 error2 = 2 * error;
        if (error2 >= distance_y){
            error += distance_y;
            pos.x += step_x;
        }
        if (error2 <= distance_x){
            error += distance_x;
            pos.y += step_y;
        }
    }
}

static void
draw_segment(RenderBuffer *render_buffer, v2 p0, v2 p1, RGBA color){
    p0 = round_v2(p0);
    p1 = round_v2(p1);

    f32 distance_x =  abs_f32(p1.x - p0.x);
    f32 distance_y = -abs_f32(p1.y - p0.y);
    f32 step_x = p0.x < p1.x ? 1.0f : -1.0f;
    f32 step_y = p0.y < p1.y ? 1.0f : -1.0f;

    f32 error = distance_x + distance_y;

    for(;;){
        if(p0.x == p1.x && p0.y == p1.y) break;
        draw_pixel(render_buffer, p0, color); // NOTE: before break, so you can draw a single point draw_segment (a pixel)

        f32 error2 = 2 * error;
        if (error2 >= distance_y){
            error += distance_y;
            p0.x += step_x;
        }
        if (error2 <= distance_x){
            error += distance_x;
            p0.y += step_y;
        }
    }
}

static void
draw_flattop_triangle(RenderBuffer *render_buffer, v2 p0, v2 p1, v2 p2, RGBA color){
    f32 left_slope = (p0.x - p2.x) / (p0.y - p2.y);
    f32 right_slope = (p1.x - p2.x) / (p1.y - p2.y);

    //s32 start_y = (int)round(p2.y);
    //s32 end_y = (int)round(p0.y);
    s32 start_y = (s32)round_f32_s32(p2.y);
    s32 end_y = (s32)round_f32_s32(p0.y);

    for(s32 y=start_y; y < end_y; ++y){
        f32 x0 = left_slope * (y + 0.5f - p0.y) + p0.x;
        f32 x1 = right_slope * (y + 0.5f - p1.y) + p1.x;
        s32 start_x = (s32)ceil(x0 - 0.5f);
        s32 end_x = (s32)ceil(x1 - 0.5f);
        for(s32 x=start_x; x < end_x; ++x){
            draw_pixel(render_buffer, (v2){(f32)x, (f32)y}, color);
        }
    }
}

static void
draw_flatbottom_triangle(RenderBuffer *render_buffer, v2 p0, v2 p1, v2 p2, RGBA color){
    f32 left_slope = (p1.x - p0.x) / (p1.y - p0.y);
    f32 right_slope = (p2.x - p0.x) / (p2.y - p0.y);

    //s32 start_y = (int)round(p0.y);
    //s32 end_y = (int)round(p1.y);
    s32 start_y = (s32)round_f32_s32(p0.y);
    s32 end_y = (s32)round_f32_s32(p1.y);

    for(s32 y=start_y; y >= end_y; --y){
        f32 x0 = left_slope * (y + 0.5f - p0.y) + p0.x;
        f32 x1 = right_slope * (y + 0.5f - p0.y) + p0.x;
        s32 start_x = (s32)ceil(x0 - 0.5f);
        s32 end_x = (s32)ceil(x1 - 0.5f);
        for(s32 x=start_x; x < end_x; ++x){
            draw_pixel(render_buffer, (v2){(f32)x, (f32)y}, color);
        }
    }
}

static void
draw_triangle_outlined(RenderBuffer *render_buffer, v2 p0, v2 p1, v2 p2, RGBA color, RGBA c_outlined, bool fill){
    if(p0.y < p1.y){ swap_v2(&p0, &p1); }
    if(p0.y < p2.y){ swap_v2(&p0, &p2); }
    if(p1.y < p2.y){ swap_v2(&p1, &p2); }

    if(fill){
        if(p0.y == p1.y){
            if(p0.x > p1.x){ swap_v2(&p0, &p1); }
            draw_flattop_triangle(render_buffer, p0, p1, p2, color);
        }
        else if(p1.y == p2.y){
            if(p1.x > p2.x){ swap_v2(&p1, &p2); }
            draw_flatbottom_triangle(render_buffer, p0, p1, p2, color);
        }
        else{
            f32 x = p0.x + ((p1.y - p0.y) / (p2.y - p0.y)) * (p2.x - p0.x);
            v2 p3 = {x, p1.y};
            if(p1.x > p3.x){ swap_v2(&p1, &p3); }
            draw_flattop_triangle(render_buffer, p1, p3, p2, color);
            draw_flatbottom_triangle(render_buffer, p0, p1, p3, color);
        }
    }
    draw_segment(render_buffer, p0, p1, c_outlined);
    draw_segment(render_buffer, p1, p2, c_outlined);
    draw_segment(render_buffer, p2, p0, c_outlined);
}

static void
draw_triangle(RenderBuffer *render_buffer, v2 p0, v2 p1, v2 p2, RGBA color, bool fill){
    if(p0.y < p1.y){ swap_v2(&p0, &p1); }
    if(p0.y < p2.y){ swap_v2(&p0, &p2); }
    if(p1.y < p2.y){ swap_v2(&p1, &p2); }

    if(fill){
        if(p0.y == p1.y){
            if(p0.x > p1.x){ swap_v2(&p0, &p1); }
            draw_flattop_triangle(render_buffer, p0, p1, p2, color);
        }
        else if(p1.y == p2.y){
            if(p1.x > p2.x){ swap_v2(&p1, &p2); }
            draw_flatbottom_triangle(render_buffer, p0, p1, p2, color);
        }
        else{
            f32 x = p0.x + ((p1.y - p0.y) / (p2.y - p0.y)) * (p2.x - p0.x);
            v2 p3 = {x, p1.y};
            if(p1.x > p3.x){ swap_v2(&p1, &p3); }
            draw_flattop_triangle(render_buffer, p1, p3, p2, color);
            draw_flatbottom_triangle(render_buffer, p0, p1, p3, color);
        }
    }
    else{
        draw_segment(render_buffer, p0, p1, color);
        draw_segment(render_buffer, p1, p2, color);
        draw_segment(render_buffer, p2, p0, color);
    }
}

static void
clear_color(RenderBuffer *render_buffer, RGBA color={0, 0, 0, 1}){
    u32 *pixel = (u32 *)((u8 *)(render_buffer->base));
    for(s32 i=0; i < (render_buffer->width * render_buffer->height); ++i){
        u32 new_color = (round_f32_s32(color.a * 255.0f) << 24 | round_f32_s32(color.r*255.0f) << 16 | round_f32_s32(color.g*255.0f) << 8 | round_f32_s32(color.b*255.0f) << 0);
        *pixel++ = new_color;
    }
}

// TODO: clear only a region
// INCOMPLETE: Rect type has changed
//static void
//clear_color_region(RenderBuffer *render_buffer, Rect rect, RGBA color={0, 0, 0, 1}){
//    v2s32 pos = round_v2_v2s32(rect.pos);
//    u8 *row = (u8 *)render_buffer->base +
//                 ((render_buffer->height - pos.y - 1) * render_buffer->stride) +
//                 (pos.x * render_buffer->bytes_per_pixel);
//
//    for(s32 y=rect.y0; y < (rect.y0 + rect.y1); ++y){
//        u32* pixel = (u32*)row;
//        for(s32 x=rect.x0; x < (rect.x0 + rect.x1); ++x){
//            u32 new_color = (round_f32_s32(color.a * 255.0f) << 24 | round_f32_s32(color.r*255.0f) << 16 | round_f32_s32(color.g*255.0f) << 8 | round_f32_s32(color.b*255.0f) << 0);
//            *pixel++ = new_color;
//        }
//        row += render_buffer->stride;
//    }
//}


// UNTESTED: untested with rect screenspace change
static void
draw_bitmap_clip(RenderBuffer *render_buffer, v2 pos, Bitmap image, v4 clip_region){
    //RectPixelSpace rect_ps = screen_to_pixel(rect, resolution);
    Rect rect = make_rect(pos.x, pos.y, pos.x + image.width, pos.y + image.height);
    v2s32 pixel_min = round_v2_v2s32(rect.min);
    v2s32 pixel_max = round_v2_v2s32(rect.max);

    if(clip_region == (v4){0,0,0,0}){
        u32* at = (u32*)image.pixels;
        for(f32 y=pixel_min.y; y < pixel_max.y; ++y){
            for(f32 x=pixel_min.x; x < pixel_max.x; ++x){
                RGBA color = u32_to_rgba(*at++);
                draw_pixel(render_buffer, (v2){x, y}, color);
            }
        }
    }
    else{
        Rect cr = make_rect(.1f, .15f, .2f, .25f);

        if(pixel_min.x < cr.x0){
            pixel_min.x = cr.x0;
            pixel_max.x = image.width - (cr.x0 - pixel_min.x);
        }
        if((pixel_min.x + pixel_max.x) > (cr.x0 + cr.x1)){
            pixel_max.x = pixel_max.x - ((pixel_min.x + pixel_max.x) - (cr.x0 + cr.x1));
        }

        if(pixel_min.y < cr.y0){
            pixel_min.y = cr.y0;
            pixel_max.y = image.height - (cr.y0 - pixel_min.y);
        }
        if((pixel_min.y + pixel_max.y) > (cr.y0 + cr.y1)){
            pixel_max.y = pixel_max.y - ((pixel_min.y + pixel_max.y) - (cr.y0 + cr.y1));
        }

        u32 rounded_x = round_f32_u32(pixel_min.x);
        u32 rounded_y = round_f32_u32(pixel_min.y);

        // clamp is wrong here, was 1 n 0 now 0 n 1
        u32 x_shift = (u32)clamp_f32(0, (cr.x0 - pixel_min.x), 100000);
        u32 y_shift = (u32)clamp_f32(0, (cr.y0 - pixel_min.y), 100000);

        u32 iy = 0;
        for(u32 y = rounded_y; y < rounded_y + pixel_max.y; ++y){
            u32 ix = 0;
            for(u32 x = rounded_x; x < rounded_x + pixel_max.x; ++x){
                u8 *byte = (u8 *)image.pixels + ((y_shift + iy) * image.width * 4) + ((x_shift + ix) * 4);
                u32 *c = (u32 *)byte;
                RGBA color = u32_to_rgba(*c);
                draw_pixel(render_buffer, (v2){(f32)x, (f32)y}, color);
                ix++;
            }
            iy++;
        }
    }
}

// UNTESTED: untested with rect screenspace change
static void
draw_bitmap(RenderBuffer *render_buffer, v2 pos, Bitmap image){
    draw_bitmap_clip(render_buffer, pos, image, (v4){0,0,0,0});
}

static void
draw_rect_slow(RenderBuffer *render_buffer, Rect rect, RGBA color){
    //RectPixelSpace rect_ps = screen_to_pixel(rect, resolution);

    for(f32 y=rect.y0; y <= rect.y1; ++y){
        for(f32 x=rect.x0; x <= rect.x1; ++x){
            draw_pixel(render_buffer, (v2){x, y}, color);
        }
    }
}

static void
draw_rect(RenderBuffer *render_buffer, Rect rect, RGBA color){
    // round min/max
    v2s32 pixel_min = round_v2_v2s32(rect.min);
    v2s32 pixel_max = round_v2_v2s32(rect.max);

    // clamp min/max of rect to render_buffer
    if(pixel_min.x < 0) { pixel_min.x = 0; }
    if(pixel_min.y < 0) { pixel_min.y = 0; }
    if(pixel_max.x > render_buffer->width) { pixel_max.x = render_buffer->width; }
    if(pixel_max.y > render_buffer->height) { pixel_max.y = render_buffer->height; }

    // helper 4x variables
    __m128i max_x_4x = _mm_set1_epi32(pixel_max.x);
    __m128 color_255_4x = _mm_set_ps1(255.0f);
    __m128 color_one_4x = _mm_set_ps1(1.0f);
    __m128 color_zero_4x = _mm_set_ps1(0.0f);
    __m128 mask_FF = _mm_set1_epi32(0xFF);

    // 4x of input color
    __m128 color_a_4x = _mm_set_ps1(color.a);
    __m128 color_r_4x = _mm_set_ps1(color.r);
    __m128 color_g_4x = _mm_set_ps1(color.g);
    __m128 color_b_4x = _mm_set_ps1(color.b);

    // pre multiply alphas for current
    __m128 current_a_percent_4x = _mm_sub_ps(color_one_4x, color_a_4x);
    __m128 scaled_color_r_4x = _mm_mul_ps(color_r_4x, color_255_4x);
    __m128 scaled_color_g_4x = _mm_mul_ps(color_g_4x, color_255_4x);
    __m128 scaled_color_b_4x = _mm_mul_ps(color_b_4x, color_255_4x);

    // pre multiply alphas for input color
    __m128 new_color_r_4x = (color_a_4x * scaled_color_r_4x);
    __m128 new_color_g_4x = (color_a_4x * scaled_color_g_4x);
    __m128 new_color_b_4x = (color_a_4x * scaled_color_b_4x);

    // compute initial increment
    s32 width = pixel_max.x - pixel_min.x;
    s32 remainder = width % 4;
    s32 increment = remainder ? remainder : 4;

    // compute mask
    __m128i mask = _mm_setr_epi32(-(increment > 0), -(increment > 1), -(increment > 2), -(increment > 3));
    __m128i ones_mask = _mm_cmpeq_epi32(mask, mask);

    // get row based of clamped pixel_min.x/pixel_min.y
    u8 *row = (u8 *)render_buffer->base +
              ((render_buffer->height - 1 - pixel_min.y) * render_buffer->stride) +
              (pixel_min.x * render_buffer->bytes_per_pixel);

    // iterate over clamped pixel_min.x/pixel_min.y
    //BEGIN_CYCLE_COUNTER(draw_rect_fast)
    for(s32 y=pixel_min.y; y < pixel_max.y; ++y){
        u32* pixel = (u32*)row;

        for(s32 x=pixel_min.x; x < pixel_max.x; ){
            // if x in dead zone, load last 4 pixels and set mask appropriately
            if(x > render_buffer->width - 4){
                row = (u8 *)render_buffer->base +
                      ((render_buffer->height - 1 - pixel_min.y) * render_buffer->stride) +
                      ((render_buffer->width - 4) * render_buffer->bytes_per_pixel);
                pixel = (u32*)row;
                mask = ~mask;
            }

            __m128i loaded_pixel_4x = _mm_loadu_si128((__m128i*)pixel);

            // from: argb-argb-argb-argb
            // to:   aaaa-rrrr-gggg-bbbb
            __m128 loaded_r_4x = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(loaded_pixel_4x, 16), mask_FF));
            __m128 loaded_g_4x = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(loaded_pixel_4x, 8),  mask_FF));
            __m128 loaded_b_4x = _mm_cvtepi32_ps(_mm_and_si128(loaded_pixel_4x, mask_FF));

            // linear blend between loaded color and input color
            __m128 new_loaded_r_4x = _mm_mul_ps(current_a_percent_4x, loaded_r_4x);
            __m128 new_loaded_g_4x = _mm_mul_ps(current_a_percent_4x, loaded_g_4x);
            __m128 new_loaded_b_4x = _mm_mul_ps(current_a_percent_4x, loaded_b_4x);

            __m128 new_r_4x = _mm_add_ps(new_loaded_r_4x, new_color_r_4x);
            __m128 new_g_4x = _mm_add_ps(new_loaded_g_4x, new_color_g_4x);
            __m128 new_b_4x = _mm_add_ps(new_loaded_b_4x, new_color_b_4x);
            __m128 new_a_4x = _mm_mul_ps(color_a_4x, color_255_4x);

            // convert to int, cvtps rounds to nearest. no need for +0.5f
            __m128i int_a_4x = _mm_cvtps_epi32(new_a_4x);
            __m128i int_r_4x = _mm_cvtps_epi32(new_r_4x);
            __m128i int_g_4x = _mm_cvtps_epi32(new_g_4x);
            __m128i int_b_4x = _mm_cvtps_epi32(new_b_4x);

            // shift argb order
            __m128i shifted_int_a_4x = _mm_slli_epi32(int_a_4x, 24);
            __m128i shifted_int_r_4x = _mm_slli_epi32(int_r_4x, 16);
            __m128i shifted_int_g_4x = _mm_slli_epi32(int_g_4x, 8);
            __m128i shifted_int_b_4x = int_b_4x;

            // or everything together
            __m128i output_pixel_4x = _mm_or_si128(shifted_int_a_4x,
                                      _mm_or_si128(shifted_int_r_4x,
                                      _mm_or_si128(shifted_int_g_4x,
                                                   shifted_int_b_4x)));

            __m128i output_masked = _mm_and_si128(mask, output_pixel_4x);

            _mm_storeu_si128((__m128i * )pixel, output_masked);

            pixel += increment;
            x+=increment;
            increment = 4;
            mask = ones_mask;
        }
        increment = remainder ? remainder : 4;
        mask = _mm_setr_epi32(-(increment > 0), -(increment > 1), -(increment > 2), -(increment > 3));
        row -= render_buffer->stride;
    }
    //END_CYCLE_COUNTER(draw_rect_fast)
}

// UNTESTED: untested with rect screenspace change
static void
draw_box(RenderBuffer *render_buffer, Rect rect, RGBA color){
    //RectPixelSpace rect_ps = screen_to_pixel(rect, resolution);

    v2 p0 = rect.min;
    v2 p1 = {rect.max.x, rect.min.y};
    v2 p2 = rect.max;
    v2 p3 = {rect.min.x, rect.max.y};

    draw_segment(render_buffer, p0, p1, color);
    draw_segment(render_buffer, p1, p2, color);
    draw_segment(render_buffer, p2, p3, color);
    draw_segment(render_buffer, p3, p0, color);
}

static void
draw_quad(RenderBuffer *render_buffer, v2 p0_, v2 p1_, v2 p2_, v2 p3_, RGBA color, bool fill){
    v2 p0 = p0_;
    v2 p1 = p1_;
    v2 p2 = p2_;
    v2 p3 = p3_;

    if(p0.x > p1.x){ swap_v2(&p0, &p1); }
    if(p0.x > p2.x){ swap_v2(&p0, &p2); }
    if(p0.y > p2.y){ swap_v2(&p0, &p2); }
    if(p0.x > p3.x){ swap_v2(&p0, &p3); }
    if(p0.y > p3.y){ swap_v2(&p0, &p3); }

    if(p1.x < p0.x){ swap_v2(&p1, &p0); }
    if(p1.x < p3.x){ swap_v2(&p1, &p3); }
    if(p1.y > p2.y){ swap_v2(&p1, &p2); }
    if(p1.y > p3.y){ swap_v2(&p1, &p3); }

    if(p2.x < p0.x){ swap_v2(&p2, &p0); }
    if(p2.x < p3.x){ swap_v2(&p2, &p3); }
    if(p2.y < p0.y){ swap_v2(&p2, &p0); }
    if(p2.y < p1.y){ swap_v2(&p2, &p1); }

    if(p3.x > p1.x){ swap_v2(&p3, &p1); }
    if(p3.x > p2.x){ swap_v2(&p3, &p2); }
    if(p3.y < p0.y){ swap_v2(&p3, &p0); }
    if(p3.y < p1.y){ swap_v2(&p3, &p1); }

    draw_triangle(render_buffer, p0, p1, p2, color, fill);
    draw_triangle(render_buffer, p0, p2, p3, color, fill);
}

static void
draw_polygon(RenderBuffer *render_buffer, v2 *points, u32 count, RGBA color){
    v2 first = *points++;
    v2 prev = first;
    for(u32 i=1; i<count; ++i){
        draw_segment(render_buffer, prev, *points, color);
        prev = *points++;
    }
    draw_segment(render_buffer, first, prev, color);
}

static void
draw_circle(RenderBuffer *render_buffer, f32 xm, f32 ym, f32 r, RGBA color, bool fill) {
    f32 x = -r;
    f32 y = 0;
    f32 err = 2-2*r;
    do {
        draw_pixel(render_buffer, (v2){(xm - x), (ym + y)}, color);
        draw_pixel(render_buffer, (v2){(xm - y), (ym - x)}, color);
        draw_pixel(render_buffer, (v2){(xm + x), (ym - y)}, color);
        draw_pixel(render_buffer, (v2){(xm + y), (ym + x)}, color);
        r = err;
        if (r <= y){
            if(fill){
                if(ym + y == ym - y){
                    draw_segment(render_buffer, (v2){(xm - x - 1), (ym + y)}, (v2){(xm + x), (ym + y)}, color);
                }
                else{
                    draw_segment(render_buffer, (v2){(xm - x - 1), (ym + y)}, (v2){(xm + x), (ym + y)}, color);
                    draw_segment(render_buffer, (v2){(xm - x - 1), (ym - y)}, (v2){(xm + x), (ym - y)}, color);
                }
            }
            y++;
            err += y * 2 + 1;
        }
        if (r > x || err > y){
            x++;
            err += x * 2 + 1;
        }
    } while (x < 0);
}








//typedef struct Rect2{
//    u32 l, r, t, b;
//} Rect2;
//
//
//
//static Rect
//rect2_intersection(Rect a, Rect b){
//    if(a.l < b.l) a.l = b.l;
//    if(a.r < b.r) a.r = b.r;
//    if(a.t < b.t) a.t = b.t;
//    if(a.b < b.b) a.b = b.b;
//    return(a);
//}
//
//static bool
//rect2_contains_v2(Rect a, v2 b){
//    bool result = (a.l <= b.x && a.r > b.x && a.t > b.y && a.b <= b.y);
//    return(result);
//}
//
//static bool
//rect2_is_valid(Rect a){
//    bool result = (a.r < a.l && a.t > a.b);
//    return(result);
//}
//
//static bool
//rect2_cmp(Rect a, Rect b){
//    bool result = (a.l == b.l && a.r == b.r && a.t == b.t && a.b == b.b);
//    return(result);
//}
//
//// Compute the smallest rectangle containing both of the input rectangles.
//static Rect
//rect2_bounding(Rect a, Rect b){
//    Rect2 result = ZERO_INIT;
//    if(a.l > b.l) a.l = b.l;
//    if(a.r < b.r) a.r = b.r;
//    if(a.t < b.t) a.t = b.t;
//    if(a.b > b.b) a.b = b.b;
//    return(result);
//}

#endif
