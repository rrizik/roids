#ifndef CAMERA_C
#define CAMERA_C

static void
init_camera(Camera* camera){
    camera->up = make_v3(0.0f, 1.0f, 0.0f);
    camera->forward = make_v3(0.0f, 0.0f, 1.0f); // note: left handed forward
    camera->pos = make_v3(0.0f, 0.0f, 0.f);

    camera->fov = 90.0f;
    camera->rotation_speed = 30.0f;
    camera->move_speed = 40.0f;

    camera->yaw = (f32)atan2(camera->forward.x, camera->forward.z);
    camera->pitch = (f32)asin(-camera->forward.y);
}

static void
camera_update(Camera* camera, f32 dx, f32 dy, f32 dt){
    camera->yaw += dx * camera->rotation_speed;
    camera->pitch += dy * camera->rotation_speed;

    // clamp campera at top and bottom so you do the spin rotation thing
    if(camera->pitch > 89.0f){ camera->pitch = 89.0f; }
    if(camera->pitch < -89.0f){ camera->pitch = -89.0f; }

    // get forward dir
    v3 forward_dir;
    forward_dir.x = cos_f32(rad_from_deg(camera->pitch)) * sin_f32(rad_from_deg(camera->yaw));
    forward_dir.y = sin_f32(rad_from_deg(camera->pitch));
    forward_dir.z = cos_f32(rad_from_deg(camera->pitch)) * cos_f32(rad_from_deg(camera->yaw));

    // normalize and set forward dir
    camera->forward = normalize_v3(forward_dir);
}

static void
init_camera_2d(Camera2D* camera, v2 pos, f32 size){
    camera->pos = pos;
    camera->size = size;
}

static void
camera_2d_update(Camera2D* camera, f32 aspect_ratio){
    camera->right_border =  camera->x + (camera->size * aspect_ratio);
    camera->left_border =   camera->x - (camera->size * aspect_ratio);
    camera->top_border =    camera->y + (camera->size);
    camera->bottom_border = camera->y - (camera->size);
}

#endif
