#ifndef UI_C
#define UI_C

static void
init_ui(Arena* arena){
    ui_arena = arena;
    //ui_box_null = {0};
    //ui_box_top->v = &ui_box_null;
    //ui_pos_x_top->v = 0;
    //ui_pos_y_top->v = 0;
    //ui_size_w_top->v = ui_size_pixel(0, 0);
    //ui_size_h_top->v = ui_size_pixel(0, 0);
    //ui_background_color_top->v = make_RGBA(0, 0, 0, 0);
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
    UI_Box* result = push_struct(ui_arena, UI_Box);

    // todo: this needs to use linked list macros
    if(ui_box_top != 0){
        UI_Box* top_box = ui_top_box();
        result->parent = top_box;
        if(top_box->first == 0){
            top_box->first = result;
            top_box->last = result;
        }
        else{
            top_box->last->next = result;
            result->prev = top_box->last;
            top_box->last = result;
        }
    }

    result->flags = flags;
    result->string = str;

    result->pos[Axis_X] = ui_top_pos_x();
    result->pos[Axis_Y] = ui_top_pos_y();
    result->semantic_size[Axis_X] = ui_top_size_w();
    result->semantic_size[Axis_Y] = ui_top_size_h();
    result->layout_axis = ui_top_layout_axis();
    result->background_color = ui_top_background_color();
    return(result);
}

static bool
ui_layout(String8 str){
    u32 flags = UI_BoxFlag_Clickable |
                UI_BoxFlag_DrawBorder |
                UI_BoxFlag_DrawBackground |
                UI_BoxFlag_DrawDropShadow;
    UI_Box* box = ui_make_box(str, flags);

    return(true);
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
ui_traverse_text(UI_Box* node, Axis axis){
    if(node == 0){
        return;
    }

    if(node->semantic_size[axis].type == UI_SizeType_TextContent){
        if(axis == Axis_X){
            f32 width = font_string_width(0, node->string);
            //node->size[axis] = node->semantic_size[axis].value;
            node->size[axis] = width;
        }
        if(axis == Axis_Y){
            s32 height = font_vertical_offset(0);
            node->size[axis] = (f32)height;
        }
    }

    if(node->first != 0){
        ui_traverse_text(node->first, axis);
    }

    ui_traverse_text(node->next, axis);
}

static void
ui_traverse_pixel(UI_Box* node, Axis axis){
    if(node == 0){
        return;
    }

    if(node->semantic_size[axis].type == UI_SizeType_Pixel){
        node->size[axis] = node->semantic_size[axis].value;
    }

    if(node->first != 0){
        ui_traverse_pixel(node->first, axis);
    }

    ui_traverse_pixel(node->next, axis);
}

static void
ui_traverse_children(UI_Box* node, Axis axis){
    if(node == 0){
        return;
    }

    if(node->last != 0){
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
    }

    ui_traverse_children(node->prev, axis);
}

static void
ui_traverse_position_nodes(UI_Box* node, Axis axis){
    if(node == 0){
        return;
    }

    if(!node->prev){
        if(node->layout_axis == axis){
            f32 position = 0;
            for(UI_Box* sibling = node; sibling != 0; sibling = sibling->next){
                sibling->rel_pos[axis] = position;
                if(sibling->parent){
                    sibling->pos[axis] = sibling->parent->pos[axis] + sibling->rel_pos[axis];
                }
                position += sibling->size[axis];
            }
        }
        else{
            for(UI_Box* sibling = node; sibling != 0; sibling = sibling->next){
                sibling->rel_pos[axis] = 0;
            }
        }
    }

    if(node->first != 0){
        ui_traverse_position_nodes(node->first, axis);
    }

    ui_traverse_position_nodes(node->next, axis);
}

static void
ui_traverse_construct_rects(UI_Box* node){
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

    draw_quad(node->rect, node->background_color);
    if(has_flags(node->flags, UI_BoxFlag_DrawText)){
        // todo: Fucking... yuck
        f32 width = font_string_width(0, node->string);
        s32 vertical_offset = font_vertical_offset(0);
        s32 ascent = font_ascent(0);
        s32 descent = font_descent(0);

        s32 center = ascent - (ascent - descent)/2;
        v2 pos = make_v2(node->rect.min.x + node->size[Axis_X]/2 - width/2,
                         node->rect.min.y + node->size[Axis_Y]/2 + (f32)center);
        draw_text(0, node->string, pos, MAGENTA);
    }

    if(node->first != 0){
        ui_traverse_construct_rects(node->first);
    }

    ui_traverse_construct_rects(node->next);
}

//static void
//ui_traverse(UI_Box* node){
//    if(node == 0){
//        return;
//    }
//
//    if(node->first != 0){
//        ui_traverse(node->first);
//    }
//
//    print("node: %s\n", node->string.str);
//
//    ui_traverse(node->next);
//}
//
//static void
//ui_traverse_reverse(UI_Box* node){
//    if(node == 0){
//        return;
//    }
//
//    if(node->last != 0){
//        ui_traverse_reverse(node->last);
//    }
//
//    print("node: %s\n", node->string.str);
//
//    ui_traverse_reverse(node->prev);
//}

static void
ui_reset_stacks(){
    ui_box_top = 0;
    ui_pos_x_top = 0;
    ui_pos_y_top = 0;
    ui_size_w_top = 0;
    ui_size_h_top = 0;
    ui_layout_axis_top = 0;
    ui_background_color_top = 0;
}

#endif
