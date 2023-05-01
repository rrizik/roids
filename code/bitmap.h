#ifndef BITMAP_H
#define BITMAP_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
    String8 full_path = str8_concatenate(scratch.arena, dir, file);

    int x,y,n;
    int ok = stbi_info((char const*)full_path.str, &x, &y, &n);
    result.base = (u8*)stbi_load((char const*)full_path.str, &result.width, &result.width, &result.stride, 0);
//    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
    result.stride = result.width * result.stride;

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

// CONSIDER: do we need to pass in arena here? and if we do, why don't we use it to allocate an arena type instead of using it for os_file_read()
static Bitmap
load_bitmap(Arena *arena, String8 dir, String8 file_name){
    Bitmap result = {0};

    FileData bitmap_file;
    bool succeed = os_file_read(&bitmap_file, arena, dir, file_name);
    if(succeed){
        BitmapHeader *header = (BitmapHeader *)bitmap_file.base;
        result.base = (u8 *)bitmap_file.base + header->bitmap_offset;
        result.width = header->width;
        result.height = header->height;
        result.stride = header->width * 4;

        // NOTE: As bmps can have ARGB or RGBA or ..., we need to find where our color
        // shifts are and position the each 8 bit into a ARGB format.
        BitScanResult red_shift;
        BitScanResult green_shift;
        BitScanResult blue_shift;
        BitScanResult alpha_shift;
        if(header->Compression == 3){
            u32 alpha_mask = ~(header->RedMask | header->GreenMask | header->BlueMask);
            BitScanResult red_shift = find_first_set_bit(header->RedMask);
            BitScanResult green_shift = find_first_set_bit(header->GreenMask);
            BitScanResult blue_shift = find_first_set_bit(header->BlueMask);
            BitScanResult alpha_shift = find_first_set_bit(alpha_mask);

            assert(alpha_shift.found);
            assert(red_shift.found);
            assert(green_shift.found);
            assert(blue_shift.found);
        }

        u32* pixel = (u32*)result.base;
        for(s32 y=0; y < result.height; ++y){
            for(s32 x=0; x < result.width; ++x){
                // get u32 color. shift colors out of correct location
                u32 color_u32 = *pixel;
                if(header->Compression == 3){
                    u32 color_u32 = ((((*pixel >> alpha_shift.index) & 0xFF) << 24) |
                                     (((*pixel >> red_shift.index) & 0xFF)   << 16) |
                                     (((*pixel >> green_shift.index) & 0xFF)  << 8) |
                                     (((*pixel >> blue_shift.index) & 0xFF)   << 0));
                }

                // u32 to RGBA
                RGBA color = {
                    .a = ((f32)((color_u32 >> 24) & 0xFF) / 255.0f),
                    .r = ((f32)((color_u32 >> 16) & 0xFF) / 255.0f),
                    .g = ((f32)((color_u32 >> 8) & 0xFF) / 255.0f),
                    .b = ((f32)((color_u32 >> 0) & 0xFF) / 255.0f),
                };

                // SRGB to linear
                color.r = square_f32(color.r),
                color.g = square_f32(color.g),
                color.b = square_f32(color.b),

                // gamma correction
                color.rgb *= color.a;

                // linear to SRGB
                color.r = sqrt_f32(color.r),
                color.g = sqrt_f32(color.g),
                color.b = sqrt_f32(color.b),

                // write pixel
                *pixel++ = (u32)(round_f32_u32(color.a * 255.0f) << 24 |
                                 round_f32_u32(color.r * 255.0f) << 16 |
                                 round_f32_u32(color.g * 255.0f) << 8 |
                                 round_f32_u32(color.b * 255.0f) << 0);
            }
        }
    }
    return(result);
}


#endif
