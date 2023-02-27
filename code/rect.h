#ifndef RECT_H
#define RECT_H


// IMPORTANT
// INCOMPLETE We changed our Rect type, make sure all rect functions work on screenspace
// CLEANUP

typedef struct Rect {
    union {
        struct {
            f32 x0, y0;
            f32 x1, y1;
        };
        v2 min;
        v2 max;
    };
} Rect;

static Rect
make_rect(f32 x0, f32 y0, f32 x1, f32 y1){
    Rect result;
    result.x0 = x0;
    result.y0 = y0;
    result.x1 = x1;
    result.y1 = y1;
    return(result);
}

static v2
screen_to_pixel(v2 point, v2s32 resolution){
    v2 result = {
        point.x * resolution.w,
        point.y * resolution.h
    };
    return(result);
}

static v2
rect_width_height(Rect rect){
    v2 p_min = screen_to_pixel(rect.min, resolution);
    v2 p_max = screen_to_pixel(rect.max, resolution);

    v2 result = {
        p_max.x - p_min.x,
        p_max.y - p_min.y,
    };
    return(result);
}

static f32
pixel_to_screen(f32 pixel, f32 max){
    return(pixel / max);
}

static bool
rect_collides_rect(Rect r1, Rect r2){
    if((r1.x0 < r2.x0 + r2.x1) &&
       (r1.x0 + r1.x1 > r2.x0) &&
       (r1.y0 < r2.y0 + r2.y1) &&
       (r1.y0 + r1.y1 > r2.y0)){
        return true;
    }
    return false;
}

static bool
rect_collides_point(Rect r1, v2 p){
    if((p.x < r1.x0 + r1.x1) &&
       (p.x > r1.x0) &&
       (p.y < r1.y0 + r1.y1) &&
       (p.y > r1.y0)){
        return true;
    }
    return false;
}

static bool
rect_contains_rect(Rect r1, Rect r2){
    if((r2.x0 > r1.x0) &&
       (r2.x0 + r2.x1 < r1.x0 + r1.x1) &&
       (r2.y0 > r1.y0) &&
       (r2.y0 + r2.y1 < r1.y0 + r1.y1)){
        return true;
    }
    return false;
}

static Rect
rect_get_border_extruding(Rect rect, s32 border_size){
    Rect result = {
        rect.x0 - border_size,
        rect.y0 - border_size,
        rect.x1 + (border_size * 2),
        rect.y1 + (border_size * 2),
    };
    return(result);
}

static Rect
rect_get_border_intruding(Rect rect, s32 border_size){
    Rect result = {
        rect.x0 + border_size,
        rect.y0 + border_size,
        rect.x1 - (border_size * 2),
        rect.y1 - (border_size * 2),
    };
    return(result);
}


#endif
