#ifndef CAMERA_H
#define CAMERA_H

struct Camera{
    f32 fov;

    f32 yaw;
    f32 pitch;

    v3 position;
    v3 forward;
    v3 up;

    f32 move_speed;
    f32 rotation_speed;
};
static Camera camera;

// nocheckin:
// nocheckin:
// nocheckin:
static void
init_camera(){
    camera.up = make_v3(0.0f, 1.0f, 0.0f);
    camera.forward = make_v3(0.0f, 0.0f, 1.0f); // note: left handed forward
    camera.position = make_v3(0.0f, 1.0f, -100.0f);

    camera.fov = 90.0f;
    camera.rotation_speed = 0.10f;
    camera.move_speed = 40.0f;
}

static void
camera_update(v3 position){
    camera.position = camera.position + position;
}

#endif
