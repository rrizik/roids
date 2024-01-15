#ifndef RECT_H
#define RECT_H

// NOTE: Rect by default is in screenspace
typedef union Rect {
    struct {
        f32 x0, y0;
        f32 x1, y1;
    };
    struct {
        v2 min;
        v2 max;
    };
} Rect;

static Rect make_rect(f32 x0, f32 y0, f32 x1, f32 y1);
static Rect make_rect(v2 min, v2 max);
static Rect rect_pixel_from_clip(Rect r, v2s32 res);
static Rect rect_clip_from_pixel(Rect r, v2s32 res);
static Rect rect_clip_from_pixel_inverted(Rect r, v2s32 res);

static v2 rect_width_height(Rect rect);
static bool rect_collides_rect(Rect r1, Rect r2);
static bool rect_collides_point(Rect r1, v2 p);
static bool rect_contains_rect(Rect r1, Rect r2);
static Rect rect_calc_border(Rect rect, s32 border_size);


#endif
