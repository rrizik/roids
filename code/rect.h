#ifndef RECT_H
#define RECT_H

typedef struct Rect{
    //struct{ f32 x; f32 y;} pos;
    //struct{ s32 w; s32 h;} dim;
    struct {f32 x; f32 y; f32 w; f32 h; };
    struct {f32 x0; f32 y0; f32 x1; f32 y1; };
} Rect;

static Rect
make_rect(f32 x, f32 y, f32 w, f32 h){
    Rect result = {
        x = x,
        y = y,
        w = w,
        h = h,
    };
    return(result);
}

static f32
ratio_to_pixel(f32 ratio, f32 max){
    return(ratio * max);
}

static f32
pixel_to_ratio(f32 pixel, f32 max){
    return(pixel / max);
}

static bool
rect_collides_rect(Rect r1, Rect r2){
    if((r1.x < r2.x + r2.w) &&
       (r1.x + r1.w > r2.x) &&
       (r1.y < r2.y + r2.h) &&
       (r1.y + r1.h > r2.y)){
        return true;
    }
    return false;
}

static bool
rect_collides_point(Rect r1, v2 p){
    if((p.x < r1.x + r1.w) &&
       (p.x > r1.x) &&
       (p.y < r1.y + r1.h) &&
       (p.y > r1.y)){
        return true;
    }
    return false;
}

static bool
rect_contains_rect(Rect r1, Rect r2){
    if((r2.x > r1.x) &&
       (r2.x + r2.w < r1.x + r1.w) &&
       (r2.y > r1.y) &&
       (r2.y + r2.h < r1.y + r1.h)){
        return true;
    }
    return false;
}

static Rect
rect_get_border_extruding(Rect rect, s32 border_size){
    Rect result = {
        rect.x - border_size,
        rect.y - border_size,
        rect.w + (border_size * 2),
        rect.h + (border_size * 2),
    };
    return(result);
}

static Rect
rect_get_border_intruding(Rect rect, s32 border_size){
    Rect result = {
        rect.x + border_size,
        rect.y + border_size,
        rect.w - (border_size * 2),
        rect.h - (border_size * 2),
    };
    return(result);
}


#endif
