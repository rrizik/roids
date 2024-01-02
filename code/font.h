#ifndef FONT_H
#define FONT_H

#define GLYPH_WIDTH 9
#define GLYPH_HEIGHT 16

static u64 font[] = {
	0x0000000000000000UL, 0x0000000000000000UL, 0xBD8181A5817E0000UL, 0x000000007E818199UL, 0xC3FFFFDBFF7E0000UL, 0x000000007EFFFFE7UL, 0x7F7F7F3600000000UL, 0x00000000081C3E7FUL,
	0x7F3E1C0800000000UL, 0x0000000000081C3EUL, 0xE7E73C3C18000000UL, 0x000000003C1818E7UL, 0xFFFF7E3C18000000UL, 0x000000003C18187EUL, 0x3C18000000000000UL, 0x000000000000183CUL,
	0xC3E7FFFFFFFFFFFFUL, 0xFFFFFFFFFFFFE7C3UL, 0x42663C0000000000UL, 0x00000000003C6642UL, 0xBD99C3FFFFFFFFFFUL, 0xFFFFFFFFFFC399BDUL, 0x331E4C5870780000UL, 0x000000001E333333UL,
	0x3C666666663C0000UL, 0x0000000018187E18UL, 0x0C0C0CFCCCFC0000UL, 0x00000000070F0E0CUL, 0xC6C6C6FEC6FE0000UL, 0x0000000367E7E6C6UL, 0xE73CDB1818000000UL, 0x000000001818DB3CUL,
	0x1F7F1F0F07030100UL, 0x000000000103070FUL, 0x7C7F7C7870604000UL, 0x0000000040607078UL, 0x1818187E3C180000UL, 0x0000000000183C7EUL, 0x6666666666660000UL, 0x0000000066660066UL,
	0xD8DEDBDBDBFE0000UL, 0x00000000D8D8D8D8UL, 0x6363361C06633E00UL, 0x0000003E63301C36UL, 0x0000000000000000UL, 0x000000007F7F7F7FUL, 0x1818187E3C180000UL, 0x000000007E183C7EUL,
	0x1818187E3C180000UL, 0x0000000018181818UL, 0x1818181818180000UL, 0x00000000183C7E18UL, 0x7F30180000000000UL, 0x0000000000001830UL, 0x7F060C0000000000UL, 0x0000000000000C06UL,
	0x0303000000000000UL, 0x0000000000007F03UL, 0xFF66240000000000UL, 0x0000000000002466UL, 0x3E1C1C0800000000UL, 0x00000000007F7F3EUL, 0x3E3E7F7F00000000UL, 0x0000000000081C1CUL,
	0x0000000000000000UL, 0x0000000000000000UL, 0x18183C3C3C180000UL, 0x0000000018180018UL, 0x0000002466666600UL, 0x0000000000000000UL, 0x36367F3636000000UL, 0x0000000036367F36UL,
	0x603E0343633E1818UL, 0x000018183E636160UL, 0x1830634300000000UL, 0x000000006163060CUL, 0x3B6E1C36361C0000UL, 0x000000006E333333UL, 0x000000060C0C0C00UL, 0x0000000000000000UL,
	0x0C0C0C0C18300000UL, 0x0000000030180C0CUL, 0x30303030180C0000UL, 0x000000000C183030UL, 0xFF3C660000000000UL, 0x000000000000663CUL, 0x7E18180000000000UL, 0x0000000000001818UL,
	0x0000000000000000UL, 0x0000000C18181800UL, 0x7F00000000000000UL, 0x0000000000000000UL, 0x0000000000000000UL, 0x0000000018180000UL, 0x1830604000000000UL, 0x000000000103060CUL,
	0xDBDBC3C3663C0000UL, 0x000000003C66C3C3UL, 0x1818181E1C180000UL, 0x000000007E181818UL, 0x0C183060633E0000UL, 0x000000007F630306UL, 0x603C6060633E0000UL, 0x000000003E636060UL,
	0x7F33363C38300000UL, 0x0000000078303030UL, 0x603F0303037F0000UL, 0x000000003E636060UL, 0x633F0303061C0000UL, 0x000000003E636363UL, 0x18306060637F0000UL, 0x000000000C0C0C0CUL,
	0x633E6363633E0000UL, 0x000000003E636363UL, 0x607E6363633E0000UL, 0x000000001E306060UL, 0x0000181800000000UL, 0x0000000000181800UL, 0x0000181800000000UL, 0x000000000C181800UL,
	0x060C183060000000UL, 0x000000006030180CUL, 0x00007E0000000000UL, 0x000000000000007EUL, 0x6030180C06000000UL, 0x00000000060C1830UL, 0x18183063633E0000UL, 0x0000000018180018UL,
	0x7B7B63633E000000UL, 0x000000003E033B7BUL, 0x7F6363361C080000UL, 0x0000000063636363UL, 0x663E6666663F0000UL, 0x000000003F666666UL, 0x03030343663C0000UL, 0x000000003C664303UL,
	0x66666666361F0000UL, 0x000000001F366666UL, 0x161E1646667F0000UL, 0x000000007F664606UL, 0x161E1646667F0000UL, 0x000000000F060606UL, 0x7B030343663C0000UL, 0x000000005C666363UL,
	0x637F636363630000UL, 0x0000000063636363UL, 0x18181818183C0000UL, 0x000000003C181818UL, 0x3030303030780000UL, 0x000000001E333333UL, 0x1E1E366666670000UL, 0x0000000067666636UL,
	0x06060606060F0000UL, 0x000000007F664606UL, 0xC3DBFFFFE7C30000UL, 0x00000000C3C3C3C3UL, 0x737B7F6F67630000UL, 0x0000000063636363UL, 0x63636363633E0000UL, 0x000000003E636363UL,
	0x063E6666663F0000UL, 0x000000000F060606UL, 0x63636363633E0000UL, 0x000070303E7B6B63UL, 0x363E6666663F0000UL, 0x0000000067666666UL, 0x301C0663633E0000UL, 0x000000003E636360UL,
	0x18181899DBFF0000UL, 0x000000003C181818UL, 0x6363636363630000UL, 0x000000003E636363UL, 0xC3C3C3C3C3C30000UL, 0x00000000183C66C3UL, 0xDBC3C3C3C3C30000UL, 0x000000006666FFDBUL,
	0x18183C66C3C30000UL, 0x00000000C3C3663CUL, 0x183C66C3C3C30000UL, 0x000000003C181818UL, 0x0C183061C3FF0000UL, 0x00000000FFC38306UL, 0x0C0C0C0C0C3C0000UL, 0x000000003C0C0C0CUL,
	0x1C0E070301000000UL, 0x0000000040607038UL, 0x30303030303C0000UL, 0x000000003C303030UL, 0x0000000063361C08UL, 0x0000000000000000UL, 0x0000000000000000UL, 0x0000FF0000000000UL,
	0x0000000000180C0CUL, 0x0000000000000000UL, 0x3E301E0000000000UL, 0x000000006E333333UL, 0x66361E0606070000UL, 0x000000003E666666UL, 0x03633E0000000000UL, 0x000000003E630303UL,
	0x33363C3030380000UL, 0x000000006E333333UL, 0x7F633E0000000000UL, 0x000000003E630303UL, 0x060F0626361C0000UL, 0x000000000F060606UL, 0x33336E0000000000UL, 0x001E33303E333333UL,
	0x666E360606070000UL, 0x0000000067666666UL, 0x18181C0018180000UL, 0x000000003C181818UL, 0x6060700060600000UL, 0x003C666660606060UL, 0x1E36660606070000UL, 0x000000006766361EUL,
	0x18181818181C0000UL, 0x000000003C181818UL, 0xDBFF670000000000UL, 0x00000000DBDBDBDBUL, 0x66663B0000000000UL, 0x0000000066666666UL, 0x63633E0000000000UL, 0x000000003E636363UL,
	0x66663B0000000000UL, 0x000F06063E666666UL, 0x33336E0000000000UL, 0x007830303E333333UL, 0x666E3B0000000000UL, 0x000000000F060606UL, 0x06633E0000000000UL, 0x000000003E63301CUL,
	0x0C0C3F0C0C080000UL, 0x00000000386C0C0CUL, 0x3333330000000000UL, 0x000000006E333333UL, 0xC3C3C30000000000UL, 0x00000000183C66C3UL, 0xC3C3C30000000000UL, 0x0000000066FFDBDBUL,
	0x3C66C30000000000UL, 0x00000000C3663C18UL, 0x6363630000000000UL, 0x001F30607E636363UL, 0x18337F0000000000UL, 0x000000007F63060CUL, 0x180E181818700000UL, 0x0000000070181818UL,
	0x1800181818180000UL, 0x0000000018181818UL, 0x18701818180E0000UL, 0x000000000E181818UL, 0x000000003B6E0000UL, 0x0000000000000000UL, 0x63361C0800000000UL, 0x00000000007F6363UL,
};

//static void
//draw_string(RenderBuffer* rb, v2 pos, String8 string, u32 color){
//    s32 pos_x  = round_f32_s32(pos.x);
//
//    for(u32 i=0; i < string.size; ++i){
//        u8 c = string.str[i];
//        if(c > 127){ c = '?'; }
//
//        u8 *data = (u8*)font + (c * GLYPH_HEIGHT);
//
//        u8 *row = (u8 *)rb->base +
//                   ((rb->height - (s32)pos.y - 1) * rb->stride) +
//                   ((s32)pos_x * rb->bytes_per_pixel);
//
//        for(s32 x=0; x < GLYPH_HEIGHT; ++x){
//            u32 *pixel = (u32*)row;
//            u8 byte = data[x];
//
//            for(s32 y=0; y < GLYPH_WIDTH; ++y){
//                if(byte & (1 << y)){
//                    *pixel = color;
//                }
//                pixel++;
//            }
//            row += rb->stride;
//        }
//        pos_x += GLYPH_WIDTH;
//    }
//}

// --------------------------
// stb_truetype implementation
// --------------------------

// NOTE: Don't use conversion errors on stb_truetype.h
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

    #define STB_RECT_PACK_IMPLEMENTATION
    #include "stb_rect_pack.h"
    #define STB_TRUETYPE_IMPLEMENTATION
    #include "stb_truetype.h"
    #pragma clang diagnostic pop
#endif

#if COMPILER_CL
    #pragma warning(push, 0)
    #define STB_RECT_PACK_IMPLEMENTATION
    #include "stb_rect_pack.h"
    #define STB_TRUETYPE_IMPLEMENTATION
    #include "stb_truetype.h"
    #pragma warning(pop)
#endif

typedef struct Glyph{
    Bitmap bitmap;
    s32 advance_width, lsb;
    s32 x0, y0, x1, y1;
    s32 w, h, xoff, yoff;
} Glyph;

typedef struct Font{
    f32 scale;
    s32 vertical_offset;
    s32 ascent, descent, line_gap;

    Glyph glyphs[128];
    stbtt_bakedchar cdata[96];

    String8 name;

    // new stuff
    stbtt_fontinfo info;

    RGBA color;
    s32 texture_w;
    s32 texture_h;
    s32 first_char;
    s32 num_chars;
    f32 size;
    stbtt_packedchar packed_chars[256];

    Texture2D atlas;
} Font;

typedef struct Font2{
    stbtt_fontinfo info;

    s32 first_char;
    s32 texture_w;
    s32 texture_h;
    f32 size;
    stbtt_packedchar packed_chars[256];

    Texture2D atlas;
} Font2;

static bool
load_font_ttf2(Arena* arena, String8 path, Font2* font, f32 size){
    // open file
    File file = os_file_open(path, GENERIC_READ, OPEN_EXISTING);
    assert_fh(file); // todo: replace all these asserts with if checks so it doesn't explode

    // init font
    String8 file_data =  os_file_read(arena, file);
    if(!stbtt_InitFont(&font->info, (u8*)file_data.str, 0)){
        return(false);
    }

    font->texture_w = 1024;
    font->texture_h = 1024;
    font->first_char = 32;
    font->size = size;
    s32 stride = font->texture_w * 4;
    ScratchArena scratch = begin_scratch(0);

    // u8 data as alpha
    String8 bitmap_a;
    bitmap_a.size = (u64)(font->texture_w * font->texture_h);
    bitmap_a.str = push_array(scratch.arena, u8, bitmap_a.size);

    stbtt_pack_context context;
    if (!stbtt_PackBegin(&context, bitmap_a.str, font->texture_w, font->texture_h, 0, 1, 0)) {
        return(false);
    }

    stbtt_pack_range range;
    range.chardata_for_range = font->packed_chars;
    range.array_of_unicode_codepoints = NULL; // Indicates that we are using the range
    range.first_unicode_codepoint_in_range = 32;
    range.num_chars = 95;
    range.font_size = size;

    if (!stbtt_PackFontRanges(&context, (u8*)file_data.str, 0, &range, 1)) {
        return(false);
    }

    stbtt_PackEnd(&context);

    // u32 data as rgba
    String8 bitmap_rgba;
    bitmap_rgba.size = (u64)(font->texture_w * font->texture_h * 4);
    bitmap_rgba.str = push_array(scratch.arena, u8, bitmap_rgba.size);
    u32_buffer_from_u8_buffer(&bitmap_a, &bitmap_rgba); // convert from Alpha to RBGA in order to use 1 shader

    D3D11_TEXTURE2D_DESC desc = {
        .Width = (u32)font->texture_w,
        .Height = (u32)font->texture_h,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
        .SampleDesc = {1, 0},
        .Usage = D3D11_USAGE_IMMUTABLE,
        .BindFlags = D3D11_BIND_SHADER_RESOURCE,
    };

    D3D11_SUBRESOURCE_DATA shader_data = {
        .pSysMem = bitmap_rgba.str,
        .SysMemPitch = (u32)stride,
    };

    hr = d3d_device->CreateTexture2D(&desc, &shader_data, &font->atlas.texture);
    assert_hr(hr);
    hr = d3d_device->CreateShaderResourceView(font->atlas.texture, 0, &font->atlas.view);
    assert_hr(hr);

    os_file_close(file);
    end_scratch(scratch);
    return(true);
}

static bool
load_font_ttf(Arena* arena, String8 path, Font* font){
    File file = os_file_open(path, GENERIC_READ, OPEN_EXISTING);
    assert_fh(file);

    String8 data =  os_file_read(arena, file);
    if(!stbtt_InitFont(&font->info, (u8*)data.str, 0)){
        return(false);
    }

    os_file_close(file);
    return(true);
}

// TODO: change this to load a baked bitmap, and use UV to access correct glyph
static void
load_font_glyphs(Arena* arena, Font* font, RGBA color, f32 size){
    stbtt_GetFontVMetrics(&font->info, &font->ascent, &font->descent, &font->line_gap);
    font->vertical_offset = font->ascent - font->descent + font->line_gap;
    font->scale = stbtt_ScaleForPixelHeight(&font->info, size);

    for(s32 c=' '; c<='~'; ++c){
        Glyph* glyph = font->glyphs + c;

        u8* codepoint_bitmap = stbtt_GetCodepointBitmap(&font->info, 0, font->scale, c, &glyph->w, &glyph->h, &glyph->xoff, &glyph->yoff);

        glyph->bitmap.width = glyph->w;
        glyph->bitmap.height = glyph->h;
        glyph->bitmap.stride = glyph->w * 4;
        glyph->bitmap.base = push_array(arena, u8, (u32)(glyph->w*glyph->h*4));

        // get codepoint info
        stbtt_GetCodepointHMetrics(&font->info, c, &glyph->advance_width, &glyph->lsb);
        stbtt_GetCodepointBitmapBox(&font->info, c, font->scale, font->scale, &glyph->x0,&glyph->y0,&glyph->x1,&glyph->y1);

        u8* dest_row = (u8*)glyph->bitmap.base + (glyph->h - 1) * (glyph->w * 4);
        for(s32 y=0; y < glyph->h; ++y){
            u32* dest = (u32*)dest_row;
            for(s32 x=0; x < glyph->w; ++x){
                u8 alpha = *codepoint_bitmap++;
                f32 linear_alpha = alpha/255.0f;
                RGBA new_color = {font->color.r, font->color.g, font->color.b, (f32)alpha};

                *dest++ = ((u32)((alpha) << 24) |
                          ((u32)(color.r * 255.0f) << 16) |
                          ((u32)(color.g * 255.0f) <<  8) |
                          ((u32)(color.b * 255.0f) <<  0));
            }
            dest_row -= glyph->w * 4;
        }
    }
}

static s32
string_width_in_pixels(String8 str, Font* font){
    s32 result = 0;
    for(u64 i=0; i < str.size; ++i){
        u8* c = str.str + i;
        Glyph* glyph = font->glyphs + *c;
        result += glyph->advance_width;
    }
    return(round_f32_s32((f32)result * font->scale));
}

#endif
