#ifndef UI_C
#define UI_C

static UI_Layout*
ui_make_layout(Arena* arena, v2 pos, String8 str, UI_LayoutFlags flags){
    UI_Layout* result = push_struct(arena, UI_Layout);
    if(current_layout != 0){
        result->parent = current_layout;
        if(current_layout->first == 0){
            current_layout->first = result;
            current_layout->last = result;
        }
        else{
            current_layout->last->next = result;
            result->prev = current_layout->last;
            current_layout->last = result;
        }
    }

    result->pos = pos;
    result->flags = flags;
    result->str = str;

    return(result);
}

static void
ui_select_layout(UI_Layout* layout){
    current_layout = layout;
}

static bool
ui_button(Arena* arena, String8 str){
    UI_Layout* result = push_struct(arena, UI_Layout);
    result->parent = current_layout;
    if(current_layout->first == 0){
        current_layout->first = result;
        current_layout->last = result;
    }
    else{
        current_layout->last->next = result;
        result->prev = current_layout->last;
        current_layout->last = result;
    }

    result->str = str;
    // look at current layout
    // draw based on its position
    // draw based on siblings
    // what was the event
    // act accordingly
    return(true);
}

static void
traverse_ui(UI_Layout* node){
    if(node == 0){
        return;
    }

    if(node->first != 0){
        traverse_ui(node->first);
    }

    print("node: %s\n", node->str.str);

    UI_Layout* node_next = node->next;
    traverse_ui(node_next);
}

static void
traverse_ui_reverse(UI_Layout* node){
    if(node == 0){
        return;
    }

    if(node->last != 0){
        traverse_ui_reverse(node->last);
    }

    print("node: %s\n", node->str.str);

    UI_Layout* node_prev = node->prev;
    traverse_ui_reverse(node_prev);
}

#endif
