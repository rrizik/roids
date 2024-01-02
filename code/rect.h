#ifndef RECT_H
#define RECT_H


// IMPORTANT
// INCOMPLETE We changed our Rect type, make sure all rect functions work on screenspace
// CLEANUP

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
} Rect, RectPixelSpace, RectScreenSpace;

static Rect
make_rect(f32 x0, f32 y0, f32 x1, f32 y1){
    Rect result;
    result.x0 = x0;
    result.y0 = y0;
    result.x1 = x1;
    result.y1 = y1;
    return(result);
}

static Rect
make_rect(v2 min, v2 max){
    Rect result;
    result.min = min;
    result.max = max;
    return(result);
}

static Rect
rect_clip_to_pixel(Rect r, v2s32 res){
    Rect result = {
        ((r.x0  + 1.0f) / 2.0f) * (f32)res.w,
        ((r.y0  + 1.0f) / 2.0f) * (f32)res.h,
        ((r.x1  + 1.0f) / 2.0f) * (f32)res.w,
        ((r.y1  + 1.0f) / 2.0f) * (f32)res.h,
    };
    return(result);
}

static Rect
rect_pixel_to_clip(Rect r, v2s32 res){
    Rect result = {
        ((r.x0 / (f32)res.w) * 2.0f) - 1.0f,
        ((r.y0 / (f32)res.h) * 2.0f) - 1.0f,
        ((r.x1 / (f32)res.w) * 2.0f) - 1.0f,
        ((r.y1 / (f32)res.h) * 2.0f) - 1.0f,
    };
    return(result);
}

static Rect
rect_pixel_to_clip3(Rect r, v2s32 res){
    Rect result = {
        ((r.x0 / (f32)res.w) * 2.0f) - 1.0f,
       -(((r.y0 / (f32)res.h) * 2.0f) - 1.0f),
        ((r.x1 / (f32)res.w) * 2.0f) - 1.0f,
       -(((r.y1 / (f32)res.h) * 2.0f) - 1.0f)
    };
    return(result);
}

static v2
rect_width_height(Rect rect){
    //Rect ps_rect = screen_to_pixel(rect);

    v2 result = rect.max - rect.min;
    return(result);
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
rect_calc_border(Rect rect, s32 border_size){
    //RectPixelSpace rect_ps = screen_to_pixel(rect);
    Rect result = {
        rect.x0 - (f32)border_size,
        rect.y0 - (f32)border_size,
        rect.x1 + (f32)border_size,
        rect.y1 + (f32)border_size,
    };

    //RectScreenSpace result = pixel_to_screen(rect_ps);
    return(result);
}

//static Rect
//rect_get_border_intruding(Rect rect, s32 border_size){
//    RectPixelSpace rect_ps = screen_to_pixel(rect);
//
//    RectScreenSpace result = pixel_to_screen(rect_ps);
//    return(result);
//}


#endif
