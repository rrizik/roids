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
} UI_Layout, UI_Button;


typedef struct LayoutNode{
    LayoutNode* next;
    UI_Layout* layout;
} LayoutNode;
LayoutNode* top_layout = 0;

static UI_Layout* ui_make_layout(Arena* arena, v2 pos, v2 size, String8 str, UI_LayoutFlags flags);
static bool ui_button(Arena* arena, String8 str);
static void ui_traverse(UI_Layout* node);
static void ui_traverse_reverse(Arena* arena, UI_Layout* node, u32 font);
static void ui_push_layout(Arena* arena, UI_Layout* layout);
static void ui_push_size_kind(UI_Size x_size, UI_Size y_size);
static void ui_pop_layout();
static UI_Layout* ui_get_top_layout();

#endif
