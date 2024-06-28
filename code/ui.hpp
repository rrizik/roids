#ifndef UI_H
#define UI_H

typedef u32 UI_SizeType;
enum {
    UI_SizeType_None,
    UI_SizeType_Pixels,
    UI_SizeType_TextContent,
    UI_SizeType_PercentOfParent,
    UI_SizeType_ChildrenSum,
    UI_SizeType_Count,
};

typedef struct UI_Size{
  UI_SizeType kind;
  f32 value;
  f32 strictness;
} UI_Size;

typedef u32 UI_LayoutFlags;
enum {
  UI_LayoutFlag_Clickable       = (1<<0),
  UI_LayoutFlag_ViewScroll      = (1<<1),
  UI_LayoutFlag_DrawText        = (1<<2),
  UI_LayoutFlag_DrawBorder      = (1<<3),
  UI_LayoutFlag_DrawBackground  = (1<<4),
  UI_LayoutFlag_DrawDropShadow  = (1<<5),
  UI_LayoutFlag_Clip            = (1<<6),
  UI_LayoutFlag_HotAnimation    = (1<<7),
  UI_LayoutFlag_ActiveAnimation = (1<<8),
};

typedef struct UI_Layout{
    UI_Layout *first;
    UI_Layout *last;
    UI_Layout *next;
    UI_Layout *prev;
    UI_Layout *parent;

    // hash links
    //UI_Widget *hash_next;
    //UI_Widget *hash_prev;

    // key+generation info
    //UI_Key key;
    //U64 last_frame_touched_index;

    UI_LayoutFlags flags;
    String8 string;
    UI_Size semantic_size[2];

    v2 pos;
    v2 pos_rel;
    v2 size; // probably remove this?
    Rect rect;

    bool hot;
    bool active;

    RGBA background_color;
    RGBA text_color;
    RGBA hot_color;
    RGBA active_color;
} UI_Layout;

static UI_Layout* ui_make_layout(Arena* arena, v2 pos, v2 size, String8 str, UI_LayoutFlags flags);
static bool ui_button(Arena* arena, String8 str);
static void ui_traverse(UI_Layout* node);
static void ui_traverse_reverse(Arena* arena, UI_Layout* node, u32 font);

#define ui_stack_push_impl(arena, type, name, v) \
    UI_##type##Node* node = push_struct(arena, UI_##type##Node); \
    node->v = v; \
    node->next = ui_##name##_top; \
    ui_##name##_top = node; \
    return(node->v);

#define ui_stack_pop_impl(type, name) \
    UI_##type##Node* popped = ui_##name##_top; \
    ui_##name##_top = ui_##name##_top->next; \
    return(popped->v);

#define ui_stack_top_impl(name) return(ui_##name##_top->v);


typedef struct UI_LayoutNode          { UI_LayoutNode* next; UI_Layout* v; }    UI_LayoutNode;
typedef struct UI_BackgroundColorNode { UI_BackgroundColorNode* next; RGBA v; } UI_BackgroundColorNode;

UI_LayoutNode*          ui_layout_top = 0;
UI_BackgroundColorNode* ui_background_color_top = 0;

static UI_Layout* ui_push_layout(Arena* arena, UI_Layout* v)     { ui_stack_push_impl(arena, Layout, layout, v) }
static RGBA       ui_push_background_color(Arena* arena, RGBA v) { ui_stack_push_impl(arena, BackgroundColor, background_color, v) }

static UI_Layout* ui_pop_layout(void)           { ui_stack_pop_impl(Layout, layout) }
static RGBA       ui_pop_background_color(void) { ui_stack_pop_impl(BackgroundColor, background_color) }

static UI_Layout* ui_top_layout(void)           { ui_stack_top_impl(layout) }
static RGBA       ui_top_background_color(void) { ui_stack_top_impl(background_color) }

#endif
//typedef struct UI_ParentNode UI_ParentNode; struct UI_ParentNode{UI_ParentNode *next; UI_Box * v;};
//typedef struct UI_FlagsNode UI_FlagsNode; struct UI_FlagsNode{UI_FlagsNode *next; UI_BoxFlags v;};
//typedef struct UI_FixedXNode UI_FixedXNode; struct UI_FixedXNode{UI_FixedXNode *next; F32 v;};
//typedef struct UI_FixedYNode UI_FixedYNode; struct UI_FixedYNode{UI_FixedYNode *next; F32 v;};
//typedef struct UI_PrefWidthNode UI_PrefWidthNode; struct UI_PrefWidthNode{UI_PrefWidthNode *next; UI_Size v;};
//typedef struct UI_PrefHeightNode UI_PrefHeightNode; struct UI_PrefHeightNode{UI_PrefHeightNode *next; UI_Size v;};
//typedef struct UI_OpacityNode UI_OpacityNode; struct UI_OpacityNode{UI_OpacityNode *next; F32 v;};
//typedef struct UI_TextColorNode UI_TextColorNode; struct UI_TextColorNode{UI_TextColorNode *next; Vec4F32 v;};
//typedef struct UI_BackgroundColorNode UI_BackgroundColorNode; struct UI_BackgroundColorNode{UI_BackgroundColorNode *next; Vec4F32 v;};
//typedef struct UI_BorderColorNode UI_BorderColorNode; struct UI_BorderColorNode{UI_BorderColorNode *next; Vec4F32 v;};
//typedef struct UI_OverlayColorNode UI_OverlayColorNode; struct UI_OverlayColorNode{UI_OverlayColorNode *next; Vec4F32 v;};
//typedef struct UI_FillColorNode UI_FillColorNode; struct UI_FillColorNode{UI_FillColorNode *next; Vec4F32 v;};
//typedef struct UI_CursorColorNode UI_CursorColorNode; struct UI_CursorColorNode{UI_CursorColorNode *next; Vec4F32 v;};
//typedef struct UI_CornerRadius00Node UI_CornerRadius00Node; struct UI_CornerRadius00Node{UI_CornerRadius00Node *next; F32 v;};
//typedef struct UI_CornerRadius01Node UI_CornerRadius01Node; struct UI_CornerRadius01Node{UI_CornerRadius01Node *next; F32 v;};
//typedef struct UI_CornerRadius10Node UI_CornerRadius10Node; struct UI_CornerRadius10Node{UI_CornerRadius10Node *next; F32 v;};
//typedef struct UI_CornerRadius11Node UI_CornerRadius11Node; struct UI_CornerRadius11Node{UI_CornerRadius11Node *next; F32 v;};
//typedef struct UI_BorderThicknessNode UI_BorderThicknessNode; struct UI_BorderThicknessNode{UI_BorderThicknessNode *next; F32 v;};
//typedef struct UI_Slice2F32Node UI_Slice2F32Node; struct UI_Slice2F32Node{UI_Slice2F32Node *next; R_Slice2F32 v;};
//typedef struct UI_FontNode UI_FontNode; struct UI_FontNode{UI_FontNode *next; F_Tag v;};
//typedef struct UI_FontSizeNode UI_FontSizeNode; struct UI_FontSizeNode{UI_FontSizeNode *next; F32 v;};
//typedef struct UI_ChildLayoutAxisNode UI_ChildLayoutAxisNode; struct UI_ChildLayoutAxisNode{UI_ChildLayoutAxisNode *next; Axis2 v;};
//typedef struct UI_HoverCursorNode UI_HoverCursorNode; struct UI_HoverCursorNode{UI_HoverCursorNode *next; OS_CursorKind v;};
//typedef struct UI_TextAlignNode UI_TextAlignNode; struct UI_TextAlignNode{UI_TextAlignNode *next; UI_TextAlignment v;};
//typedef struct UI_TextEdgePaddingNode UI_TextEdgePaddingNode; struct UI_TextEdgePaddingNode{UI_TextEdgePaddingNode *next; F32 v;};
//typedef struct UI_SeedKeyNode UI_SeedKeyNode; struct UI_SeedKeyNode{UI_SeedKeyNode *next; UI_Key v;};
//typedef struct UI_FocusHotNode UI_FocusHotNode; struct UI_FocusHotNode{UI_FocusHotNode *next; UI_FocusKind v;};
//typedef struct UI_FocusActiveNode UI_FocusActiveNode; struct UI_FocusActiveNode{UI_FocusActiveNode *next; UI_FocusKind v;};
//
