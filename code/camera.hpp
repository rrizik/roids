#ifndef CAMERA_H
#define CAMERA_H

typedef struct Camera{
    f32 fov;

    f32 yaw;
    f32 pitch;

    v3 pos;
    v3 forward;
    v3 up;

    f32 move_speed;
    f32 rotation_speed;
} Camera;

static void init_camera(Camera* camera);
static void update_camera(Camera* camera, f32 dx, f32 dy, f32 dt);

typedef struct Camera2D{
    union{
        struct{
            f32 x;
            f32 y;
        };
        v2 pos;
    };

    f32 size;

    f32 left_border;
    f32 right_border;
    f32 top_border;
    f32 bottom_border;
} Camera2D;
static Camera2D camera;

static void init_camera_2d(Camera2D* camera, v2 pos, f32 size);

#endif
