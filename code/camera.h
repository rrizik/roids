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

static void
init_camera(){
    camera.up = make_v3(0.0f, 1.0f, 0.0f);
    camera.forward = make_v3(0.0f, 0.0f, -1.0f); // note: left handed forward
    camera.position = make_v3(100.0f, 0.0f, 0.0f);

    camera.fov = 90.0f;
    camera.rotation_speed = 0.10f;
    camera.move_speed = 40.0f;
}

static void
update_camera(s32 dx, s32 dy, f32 dt){
    camera.yaw += (f32)dx * camera.rotation_speed;
    camera.pitch += (f32)dy * camera.rotation_speed;

    // clamp campera at top and bottom so you do the spin rotation thing
    if(camera.pitch > 89.0f){ camera.pitch = 89.0f; }
    if(camera.pitch < -89.0f){ camera.pitch = -89.0f; }

    // get forward direction
    v3 forward_direction;
    forward_direction.x = -cos_f32(deg_to_rad(camera.pitch)) * cos_f32(deg_to_rad(camera.yaw));
    forward_direction.y = sin_f32(deg_to_rad(camera.pitch));
    forward_direction.z = cos_f32(deg_to_rad(camera.pitch)) * sin_f32(deg_to_rad(camera.yaw));

    // normalize and set forward direction
    camera.forward = normalized_v3(forward_direction);
}


#endif
