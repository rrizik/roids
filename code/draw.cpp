#ifndef DRAW_C
#define DRAW_C

static RGBA
srgb_to_linear_approx(RGBA value){
    RGBA result = {
        .r = square_f32(value.r),
        .g = square_f32(value.g),
        .b = square_f32(value.b),
        .a = value.a,
    };
    return(result);
}

static RGBA
linear_to_srgb_approx(RGBA value){
    RGBA result = {
        .r = sqrt_f32(value.r),
        .g = sqrt_f32(value.g),
        .b = sqrt_f32(value.b),
        .a = value.a,
    };
    return(result);
}

static RGBA
srgb_to_linear(RGBA value){
    RGBA result = {0};
    result.a = value.a;

    if(value.r < 0.04045f){
        result.r = value.r / 12.92f;
    }
    else{
        result.r = powf(((value.r + 0.055f) / (1.055f)), 2.4f);
    }

    if(value.g < 0.04045f){
        result.g = value.g / 12.92f;
    }
    else{
        result.g = powf(((value.g + 0.055f) / (1.055f)), 2.4f);
    }

    if(value.b < 0.04045f){
        result.b = value.b / 12.92f;
    }
    else{
        result.b = powf(((value.b + 0.055f) / (1.055f)), 2.4f);
    }
    return(result);
}

static void init_render_commands(Arena* arena){
    rc_arena = arena;
}

static void
draw_clear_color(RGBA color){
    RenderCommand* command = push_struct(rc_arena, RenderCommand);
    command->type = RenderCommandType_ClearColor;
    command->color = color;
}

// note: quads are always top-left, top-right, bottom-right, bottom-left order
static void
draw_quad(v2 p0, v2 p1, v2 p2, v2 p3, RGBA color){
    RenderCommand* command = push_struct(rc_arena, RenderCommand);
    command->type = RenderCommandType_Quad;
    command->color = color;
    command->p0 = p0;
    command->p1 = p1;
    command->p2 = p2;
    command->p3 = p3;
    command->texture_id = TextureAsset_White;

    //RenderBatch *batch = render_batches.last;
    //if(batch == 0 || batch->at >= batch->count || batch->texture != texture){
    //  batch = push_array(rb_arena, RenderBatch, 1);
    //  sll_queue_push(render_batches_list.first, render_batches_list.last, batch);
    //  batch->texture = texture;
    //}
    // add data to batch

    //RenderBatch* batch = get_render_batch();
    //RGBA linear_color = srgb_to_linear(color); // gamma correction
    //batch->buffer[batch->at++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    //batch->buffer[batch->at++] = { p1, linear_color, make_v2(1.0f, 0.0f) };
    //batch->buffer[batch->at++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    //batch->buffer[batch->at++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    //batch->buffer[batch->at++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    //batch->buffer[batch->at++] = { p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_quad(v2 pos, v2 dim, RGBA color){
    RenderCommand* command = push_struct(rc_arena, RenderCommand);
    command->type = RenderCommandType_Quad;
    command->color = color;
    command->p0 = pos;
    command->p1 = make_v2(pos.x + dim.w, pos.y);
    command->p2 = make_v2(pos.x + dim.w, pos.y + dim.h);
    command->p3 = make_v2(pos.x, pos.y + dim.h);
    command->texture_id = TextureAsset_White;

    //RenderBatch* batch = get_render_batch();
    //RGBA linear_color = srgb_to_linear(color); // gamma correction
    //batch->buffer[batch->at++] = { command->p0, linear_color, make_v2(0.0f, 0.0f) };
    //batch->buffer[batch->at++] = { command->p1, linear_color, make_v2(1.0f, 0.0f) };
    //batch->buffer[batch->at++] = { command->p2, linear_color, make_v2(1.0f, 1.0f) };
    //batch->buffer[batch->at++] = { command->p0, linear_color, make_v2(0.0f, 0.0f) };
    //batch->buffer[batch->at++] = { command->p2, linear_color, make_v2(1.0f, 1.0f) };
    //batch->buffer[batch->at++] = { command->p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_quad(Rect rect, RGBA color){
    RenderCommand* command = push_struct(rc_arena, RenderCommand);
    command->type = RenderCommandType_Quad;
    command->color = color;
    command->p0 = make_v2(rect.x0, rect.y0);
    command->p1 = make_v2(rect.x1, rect.y0);
    command->p2 = make_v2(rect.x1, rect.y1);
    command->p3 = make_v2(rect.x0, rect.y1);
    command->texture_id = TextureAsset_White;

    //RenderBatch* batch = get_render_batch();
    //RGBA linear_color = srgb_to_linear(color); // gamma correction
    //batch->buffer[batch->at++] = { command->p0, linear_color, make_v2(0.0f, 0.0f) };
    //batch->buffer[batch->at++] = { command->p1, linear_color, make_v2(1.0f, 0.0f) };
    //batch->buffer[batch->at++] = { command->p2, linear_color, make_v2(1.0f, 1.0f) };
    //batch->buffer[batch->at++] = { command->p0, linear_color, make_v2(0.0f, 0.0f) };
    //batch->buffer[batch->at++] = { command->p2, linear_color, make_v2(1.0f, 1.0f) };
    //batch->buffer[batch->at++] = { command->p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_quad(Quad quad, RGBA color){
    RenderCommand* command = push_struct(rc_arena, RenderCommand);
    command->type = RenderCommandType_Quad;
    command->color = color;
    command->p0 = quad.p0;
    command->p1 = quad.p1;
    command->p2 = quad.p2;
    command->p3 = quad.p3;
    command->texture_id = TextureAsset_White;

    //RenderBatch* batch = get_render_batch();
    //RGBA linear_color = srgb_to_linear(color); // gamma correction
    //batch->buffer[batch->at++] = { command->p0, linear_color, make_v2(0.0f, 0.0f) };
    //batch->buffer[batch->at++] = { command->p1, linear_color, make_v2(1.0f, 0.0f) };
    //batch->buffer[batch->at++] = { command->p2, linear_color, make_v2(1.0f, 1.0f) };
    //batch->buffer[batch->at++] = { command->p0, linear_color, make_v2(0.0f, 0.0f) };
    //batch->buffer[batch->at++] = { command->p2, linear_color, make_v2(1.0f, 1.0f) };
    //batch->buffer[batch->at++] = { command->p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_line(v2 p0, v2 p1, f32 width, RGBA color){
    RenderCommand* command = push_struct(rc_arena, RenderCommand);
    command->type = RenderCommandType_Line;
    command->color = color;

    v2 dir = direction_v2(p0, p1);
    v2 perp = perpendicular(dir);

    command->p0 = p0;
    command->p1 = p1;
    command->p2 = command->p1 + (perp * width);
    command->p3 = command->p0 + (perp * width);
    command->texture_id = TextureAsset_White;

    //RenderBatch* batch = get_render_batch();
    //RGBA linear_color = srgb_to_linear(color); // gamma correction
    //batch->buffer[batch->at++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    //batch->buffer[batch->at++] = { p1, linear_color, make_v2(1.0f, 0.0f) };
    //batch->buffer[batch->at++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    //batch->buffer[batch->at++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    //batch->buffer[batch->at++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    //batch->buffer[batch->at++] = { p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_text(Font* font, String8 text, v2 pos, RGBA color){
    if(text.size > 1000){
        u32 a = 0;
    }
    RenderCommand* command = push_struct(rc_arena, RenderCommand);
    command->type = RenderCommandType_Text;
    command->p0 = pos;
    command->color = color;
    command->font = font;
    command->text = text;

    //RenderBatch* batch = get_render_batch();
}

static void
draw_texture(u32 texture, v2 p0, v2 p1, v2 p2, v2 p3, RGBA color){
    RenderCommand* command = push_struct(rc_arena, RenderCommand);
    command->type = RenderCommandType_Texture;
    command->color = color;
    command->p0 = p0;
    command->p1 = p1;
    command->p2 = p2;
    command->p3 = p3;
    command->texture_id = texture;
}

static void
set_texture(Texture* texture){
    texture = texture;
}

static Texture*
get_texture(void){
    return(texture);
}


#endif
