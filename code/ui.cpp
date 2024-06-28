#ifndef UI_C
#define UI_C

static UI_Layout*
ui_make_layout(Arena* arena, v2 pos, v2 size, String8 str, UI_LayoutFlags flags){
    UI_Layout* result = push_struct(arena, UI_Layout);
    if(ui_layout_top != 0){
        UI_Layout* top_layout = ui_top_layout();
        result->parent = top_layout;
        if(top_layout->first == 0){
            top_layout->first = result;
            top_layout->last = result;
        }
        else{
            top_layout->last->next = result;
            result->prev = top_layout->last;
            top_layout->last = result;
        }
    }

    result->pos = pos;
    result->size = size;
    result->rect = make_rect(pos, make_v2(pos.x + size.x, pos.y + size.y));
    result->flags = flags;
    result->string = str;

    return(result);
}

static bool
ui_button(Arena* arena, String8 str){
    UI_Layout* result = push_struct(arena, UI_Layout);
    UI_Layout* top_layout = ui_top_layout();
    result->parent = top_layout;
    if(top_layout->first == 0){
        top_layout->first = result;
        top_layout->last = result;
    }
    else{
        top_layout->last->next = result;
        result->prev = top_layout->last;
        top_layout->last = result;
    }

    result->string = str;
    result->semantic_size[0] = {
      .kind = UI_SizeType_ChildrenSum,
      .value = 0,
      .strictness = 0,
    };
    result->semantic_size[1] = {
      .kind = UI_SizeType_ChildrenSum,
      .value = 0,
      .strictness = 0,
    };
    // look at current layout
    // draw based on its position
    // draw based on siblings
    // what was the event
    // act accordingly
    return(true);
}

static void
ui_traverse(UI_Layout* node){
    if(node == 0){
        return;
    }

    if(node->first != 0){
        ui_traverse(node->first);
    }

    print("node: %s\n", node->string.str);

    UI_Layout* node_next = node->next;
    ui_traverse(node_next);
}

static void
//traverse_ui_reverse(Arena* arena, UI_Layout* node, u32 font){
ui_traverse_reverse(UI_Layout* node){
    if(node == 0){
        return;
    }

    if(node->last != 0){
        //traverse_ui_reverse(arena, node->last, font);
        ui_traverse_reverse(node->last);
    }

    print("node: %s\n", node->string.str);

    UI_Layout* node_prev = node->prev;
    //traverse_ui_reverse(arena, node_prev, font);
    ui_traverse_reverse(node_prev);
}

static UI_Size
ui_sizing_sum(){
    UI_Size size = {0};
    size.kind = UI_SizeType_ChildrenSum;

    return(size);
}

#endif
