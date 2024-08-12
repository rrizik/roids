#ifndef UI_C
#define UI_C

static void
init_ui(Arena* arena, Window* window){
    ui_state.arena = arena;
    ui_state.window = window;
}

static void
ui_begin(){
    ui_push_pos_x(0);
    ui_push_pos_y(0);
    ui_push_size_w(ui_size_pixel((f32)ui_window()->width, 0));
    ui_push_size_h(ui_size_pixel((f32)ui_window()->height, 0));
    ui_push_layout_axis(Axis_Y);

    ui_push_text_padding(0);

    ui_push_background_color(CLEAR);
    ui_push_border_thickness(0);

    ui_state.root = ui_make_box(str8_literal("papa"), 0);
    ui_push_parent(ui_state.root);
}

static void
ui_end(){
    ui_parent_top = 0;
    ui_pos_x_top = 0;
    ui_pos_y_top = 0;
    ui_size_w_top = 0;
    ui_size_h_top = 0;
    ui_layout_axis_top = 0;

    ui_text_padding_top = 0;

    ui_background_color_top = 0;
    ui_border_thickness_top = 0;
}

static Arena*
ui_arena(){
    return(ui_state.arena);
}

static Window*
ui_window(){
    return(ui_state.window);
}

static UI_Box*
ui_root(){
    return(ui_state.root);
}

static UI_Size
ui_size_make(UI_SizeType type, f32 value, f32 strictness){
    UI_Size result = {0};
    result.type = type;
    result.value = value;
    result.strictness = strictness;
    return(result);
}

static UI_Box*
ui_make_box(String8 str, UI_BoxFlags flags){
    UI_Box* result = push_struct(ui_state.arena, UI_Box);

    if(ui_parent_top != 0){
        UI_Box* top_parent = ui_top_parent();
        result->parent = top_parent;
        if(top_parent->first == 0){
            top_parent->first = result;
            top_parent->last = result;
        }
        else{
            top_parent->last->next = result;
            result->prev = top_parent->last;
            top_parent->last = result;
        }
    }

    result->flags = flags;
    result->string = str;

    result->rel_pos[Axis_X] = ui_top_pos_x();
    result->rel_pos[Axis_Y] = ui_top_pos_y();
    result->semantic_size[Axis_X] = ui_top_size_w();
    result->semantic_size[Axis_Y] = ui_top_size_h();
    result->layout_axis = ui_top_layout_axis();

    result->text_padding = ui_top_text_padding();

    result->background_color = ui_top_background_color();
    result->border_thickness = ui_top_border_thickness();
    return(result);
}

static bool
ui_button(String8 str){
    u32 flags = UI_BoxFlag_Clickable |
                UI_BoxFlag_DrawBorder |
                UI_BoxFlag_DrawBackground |
                UI_BoxFlag_HotAnimation |
                UI_BoxFlag_ActiveAnimation |
                UI_BoxFlag_DrawText;
    UI_Box* box = ui_make_box(str, flags);

    return(true);
}

static void
ui_spacer(f32 size){
    u32 flags = 0;
    ui_push_size_w(ui_size_pixel(size, 0));
    ui_push_size_h(ui_size_pixel(size, 0));
    UI_Box* box = ui_make_box(str8_literal("none"), flags);
    ui_pop_size_w();
    ui_pop_size_h();
}

static void
ui_traverse_independent(UI_Box* node, Axis axis){
    if(node == 0){
        return;
    }

    switch(node->semantic_size[axis].type){
        case UI_SizeType_Pixel:{
            node->size[axis] = node->semantic_size[axis].value;
        } break;
        case UI_SizeType_TextContent:{
            if(axis == Axis_X){
                f32 width = font_string_width(0, node->string);
                node->size[axis] = (f32)width + node->text_padding;
            }
            if(axis == Axis_Y){
                s32 height = font_vertical_offset(0);
                node->size[axis] = (f32)height + node->text_padding;
            }
        } break;
    }

    if(node->first != 0){
        ui_traverse_independent(node->first, axis);
    }

    ui_traverse_independent(node->next, axis);
}

static void
ui_traverse_children(UI_Box* node, Axis axis){
    if(node == 0){
        return;
    }

    if(node->last){
        ui_traverse_children(node->last, axis);
    }

    if(node->semantic_size[axis].type == UI_SizeType_ChildrenSum){
        f32 total = 0;
        if(node->layout_axis == axis){
            for(UI_Box* child=node->first; child !=0; child = child->next){
                total += child->size[axis];
            }
            node->size[axis] = total;
        }
        else{
            for(UI_Box* child=node->first; child !=0; child = child->next){
                total = MAX(total, child->size[axis]);
            }
            node->size[axis] = total;
        }
        node->size[axis] += node->border_thickness * 2;
    }

    ui_traverse_children(node->prev, axis);
}

// absolute position - rendering and hit testing with mouse position
// otherwise you want relative position
// there is a root node that is essentially the window, simply the (0, 0) pos and w/h
static void
ui_traverse_positions(UI_Box* node, Axis axis){
    if(node == 0){
        return;
    }

    if(!node->prev){
        if(node->layout_axis == axis){
            f32 position = 0;
            for(UI_Box* sibling = node; sibling != 0; sibling = sibling->next){
                sibling->rel_pos[axis] += position;
                if(sibling->parent){
                    sibling->pos[axis] = sibling->parent->pos[axis] + sibling->rel_pos[axis] + sibling->border_thickness;
                }
                position += sibling->size[axis];
            }
        }
        else{
            for(UI_Box* sibling = node; sibling != 0; sibling = sibling->next){
                if(sibling->parent){
                    sibling->pos[axis] = sibling->parent->pos[axis] + sibling->rel_pos[axis] + sibling->border_thickness;
                }
            }
        }
    }

    if(node->first){
        ui_traverse_positions(node->first, axis);
    }
    ui_traverse_positions(node->next, axis);
}

static void
ui_traverse_rects(UI_Box* node){
    if(node == 0){
        return;
    }

    if(node->parent){
        node->rect.min = make_v2(node->parent->pos[Axis_X] + node->rel_pos[Axis_X],
                                 node->parent->pos[Axis_Y] + node->rel_pos[Axis_Y]);
        node->rect.max = make_v2(node->parent->pos[Axis_X] + node->rel_pos[Axis_X] + node->size[Axis_X],
                                 node->parent->pos[Axis_Y] + node->rel_pos[Axis_Y] + node->size[Axis_Y]);
    }
    else{
        node->rect.min = make_v2(node->pos[Axis_X], node->pos[Axis_Y]);
        node->rect.max = make_v2(node->pos[Axis_X] + node->size[Axis_X], node->pos[Axis_Y] + node->size[Axis_Y]);
    }

    if(node->first){
        ui_traverse_rects(node->first);
    }
    ui_traverse_rects(node->next);
}

static void
ui_draw(UI_Box* node){
    if(node == 0){
        return;
    }

    if(has_flags(node->flags, UI_BoxFlag_DrawBackground)){
        draw_quad(node->rect, node->background_color);
    }
    if(has_flags(node->flags, UI_BoxFlag_DrawText)){
        f32 width = font_string_width(0, node->string);
        s32 vertical_offset = font_vertical_offset(0);
        s32 ascent = font_ascent(0);
        s32 descent = font_descent(0);

        s32 center = ascent - (ascent - descent)/2;
        v2 pos = make_v2(node->rect.min.x + node->size[Axis_X]/2 - width/2,
                         node->rect.min.y + node->size[Axis_Y]/2 + (f32)center);
        draw_text(0, node->string, pos, BLACK);
    }

    if(node->first){
        ui_draw(node->first);
    }
    ui_draw(node->next);
}

#endif
