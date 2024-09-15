#ifndef GAME_HPP
#define GAME_HPP


typedef enum WindowType{
    WindowType_Fullscreen,
    WindowType_Windowed,
} WindowType;

typedef struct Window{
    union{
        struct{
            f32 width;
            f32 height;
        };
        v2 dim;
    };
    f32 aspect_ratio;

    HWND handle;
    WindowType type;
} Window;
global Window window;

#endif

