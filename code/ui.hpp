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

// Rafiks Widget
typedef struct UI_Layout{
    UI_Layout *first;
    UI_Layout *last;
    UI_Layout *next;
    UI_Layout *prev;
    UI_Layout *parent;

    UI_Size type;
    UI_LayoutFlags flags;
    String8 str;

    //f32 value;
    v2 pos;
    v2 pos_rel;
    v2 size;
    Rect rect;

    bool hot;
    bool active;
} UI_Layout, UI_Button;

UI_Layout* current_layout;


static UI_Layout* ui_make_layout(v2 pos, UI_LayoutFlags flags, String8 str);


#endif

// Ryans Widget
//typedef struct UI_Widget2 {
//  // tree links
//  UI_Widget2 *first;
//  UI_Widget2 *last;
//  UI_Widget2 *next;
//  UI_Widget2 *prev;
//  UI_Widget2 *parent;
//
//  // hash links
//  UI_Widget2 *hash_next;
//  UI_Widget2 *hash_prev;
//
//  // key+generation info
//  //UI_Key key; todo: Unsure how to define this
//  u64 last_frame_touched_index;
//
//  // per-frame info provided by builders
//  UI_WidgetFlags flags;
//  String8 string;
//  //UI_Size semantic_size[Axis2_COUNT];
//  UI_Size semantic_size[2];
//
//  // computed every frame
//  //f32 computed_rel_position[Axis2_COUNT];
//  //f32 computed_size[Axis2_COUNT];
//  f32 computed_rel_position[2];
//  f32 computed_size[2];
//  Rect rect;
//
//  // persistent data
//  f32 hot_t;
//  f32 active_t;
//} UI_Widget2;
