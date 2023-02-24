#ifndef RECT_H
#define RECT_H

typedef struct Rect{
    f32 x;
    f32 y;
    s32 w;
    s32 h;
} Rect;

static Rect
make_rect(f32 x, f32 y, s32 w, s32 h){
    Rect result = {0};
    result.x = x;
    result.y = y;
    result.w = w;
    result.h = h;
    return(result);
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
