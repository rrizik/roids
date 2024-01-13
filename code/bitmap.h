#ifndef BITMAP_H
#define BITMAP_H

#if COMPILER_CLANG
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wconversion"
    #pragma clang diagnostic ignored "-Wall"
    #pragma clang diagnostic ignored "-Wextra"
    #pragma clang diagnostic ignored "-Wreserved-id-macro"
    #pragma clang diagnostic ignored "-Wcast-qual"
    #pragma clang diagnostic ignored "-Wshadow"
    #pragma clang diagnostic ignored "-Wdouble-promotion"
    #pragma clang diagnostic ignored "-Wreserved-id-macro"
    #pragma clang diagnostic ignored "-Wcast-qual"
    #pragma clang diagnostic ignored "-Wshadow"
    #pragma clang diagnostic ignored "-Wimplicit-fallthrough"
    #pragma clang diagnostic ignored "-Wcomma"
    #pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
    #pragma clang diagnostic ignored "-Wextra-semi-stmt"
    #define STB_IMAGE_IMPLEMENTATION
    #include "stb_image.h"
    #pragma clang diagnostic pop
#endif

#if COMPILER_CL
    #pragma warning(push, 0)
    #define STB_IMAGE_IMPLEMENTATION
    #include "stb_image.h"
    #pragma warning(pop)
#endif

// IMPORTANT
// UNTESTED We changed our Rect type to screenspace, and changed all functions. Some tested, some not.
// CLEANUP
#pragma pack(push, 1)
typedef struct BitmapHeader {
    u16 file_type;
	u32 file_size;
	u16 reserved1;
	u16 reserved2;
	u32 bitmap_offset;
	u32 header_size;
	s32 width;
	s32 height;
	u16 planes;
	u16 bits_per_pixel;
    u32 Compression;
	u32 SizeOfBitmap;
	s32 HorzResolution;
	s32 VertResolution;
	u32 ColorsUsed;
	u32 ColorsImportant;
    u32 RedMask;
    u32 GreenMask;
    u32 BlueMask;
} BitmapHeader;
#pragma pack(pop)

typedef struct Bitmap{
    u8*  base;
	s32  width;
	s32  height;
	s32  stride;
} Bitmap;

//#define STBI_MALLOC, STBI_REALLOC, and STBI_FREE to avoid using malloc,realloc,free
//#ifndef STBI_MALLOC
//#define STBI_MALLOC(sz)           malloc(sz)
//#define STBI_REALLOC(p,newsz)     realloc(p,newsz)
//#define STBI_FREE(p)              free(p)
//        STBI_ONLY_PNG
//        STBI_ONLY_BMP
static Bitmap
stb_load_image(String8 dir, String8 file){
    Bitmap result = {0};
    ScratchArena scratch = begin_scratch(0);
    String8 full_path = str8_path_append(scratch.arena, dir, file);

    int x,y,n;
    int ok = stbi_info((char const*)full_path.str, &x, &y, &n);
    result.base = (u8*)stbi_load((char const*)full_path.str, &result.width, &result.width, &result.stride, 0);
//    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
    result.stride = result.width * 4;

    end_scratch(scratch);
    return(result);
}

typedef struct BitScanResult{
    bool found;
    u32 index;
} BitScanResult;

static BitScanResult
find_first_set_bit(u32 value){
    BitScanResult result = {0};

    for(u32 i=0; i < 32; ++i){
        if(value & (1 << i)){
            result.index = i;
            result.found = true;
            break;
        }
    }
    return(result);
}

// NOTE: This is not a complete BMP loader. Use stb_image.h to load bmps instead if you want something complete
static Bitmap
load_bitmap(Arena* arena, String8 filename){
    Bitmap result = {0};

    ScratchArena scratch = begin_scratch(0);
    String8 full_path = str8_concatenate(scratch.arena, path_data, filename);
    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    assert_fh(file);

    String8 data = os_file_read(arena, file);
    BitmapHeader *header = (BitmapHeader *)data.str;
    result.base = (u8 *)data.str + header->bitmap_offset;
    result.width = header->width;
    result.height = header->height;
    result.stride = header->width * 4;

    // NOTE: As bmps can have ARGB or RGBA or ..., we need to find where our color
    // shifts are and position the each 8 bit into a ARGB format.
    BitScanResult red_shift = {0};
    BitScanResult green_shift = {0};
    BitScanResult blue_shift = {0};
    BitScanResult alpha_shift = {0};
    if(header->Compression == 3){
        u32 alpha_mask = ~(header->RedMask | header->GreenMask | header->BlueMask);
        red_shift = find_first_set_bit(header->RedMask);
        green_shift = find_first_set_bit(header->GreenMask);
        blue_shift = find_first_set_bit(header->BlueMask);
        alpha_shift = find_first_set_bit(alpha_mask);

        assert(red_shift.found);
        assert(green_shift.found);
        assert(blue_shift.found);
        assert(alpha_shift.found);
    }

    u32* pixel = (u32*)result.base;
    for(s32 y=0; y < result.height; ++y){
        for(s32 x=0; x < result.width; ++x){
            // get u32 color. shift colors out of correct location
            u32 color_u32 = *pixel;
            if(header->Compression == 3){
                color_u32 = ((((*pixel >> alpha_shift.index) & 0xFF) << 24) |
                             (((*pixel >> red_shift.index) & 0xFF)   << 16) |
                             (((*pixel >> green_shift.index) & 0xFF)  << 8) |
                             (((*pixel >> blue_shift.index) & 0xFF)   << 0));
            }

            // u32 to RGBA
            RGBA color = {
                .r = ((f32)((color_u32 >> 16) & 0xFF) / 255.0f),
                .g = ((f32)((color_u32 >> 8) & 0xFF) / 255.0f),
                .b = ((f32)((color_u32 >> 0) & 0xFF) / 255.0f),
                .a = ((f32)((color_u32 >> 24) & 0xFF) / 255.0f),
            };

            // sRGB to linear
            color.r = square_f32(color.r),
            color.g = square_f32(color.g),
            color.b = square_f32(color.b),

            // gamma correction
            color.rgb *= color.a;

            // linear to sRGB
            color.r = sqrt_f32(color.r),
            color.g = sqrt_f32(color.g),
            color.b = sqrt_f32(color.b),

            // write pixel
            *pixel++ = (u32)(round_f32_u32(color.a * 255.0f) << 24 |
                             round_f32_u32(color.r * 255.0f) << 16 |
                             round_f32_u32(color.g * 255.0f) << 8  |
                             round_f32_u32(color.b * 255.0f) << 0);
        }
    }

    end_scratch(scratch);
    os_file_close(file);
    return(result);
}

static void
u32_buffer_from_u8_buffer(String8* channel_1, String8* channel_4){
    u32* base_rgba = (u32*)channel_4->str;
    u8* base_a = (u8*)channel_1->str;

    for(s32 i=0; i < channel_1->size; ++i){
        //*base_rgba = (u32)(*base_a << 24 | *base_a << 16 | *base_a << 8  | *base_a << 0);
        *base_rgba = (u32)(*base_a << 24 | 255 << 16 | 255 << 8  | 255 << 0);
        base_rgba++;
        base_a++;
    }
}

#endif
