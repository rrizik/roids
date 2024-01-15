#ifndef FONT_CPP
#define FONT_CPP

#define GLYPH_WIDTH 9
#define GLYPH_HEIGHT 16

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

static bool load_font_ttf(Arena* arena, String8 path, Font* font, f32 size){
    // open file
    File file = os_file_open(path, GENERIC_READ, OPEN_EXISTING);
    assert_fh(file); // todo: replace all these asserts with if checks so it doesn't explode

    // init font
    String8 file_data =  os_file_read(arena, file);
    if(!stbtt_InitFont(&font->info, (u8*)file_data.str, 0)){
        return(false);
    }
    stbtt_GetFontVMetrics(&font->info, &font->ascent, &font->descent, &font->line_gap);
    font->vertical_offset = font->ascent - font->descent + font->line_gap;
    font->scale = stbtt_ScaleForPixelHeight(&font->info, size);

    font->texture_w = 1024;
    font->texture_h = 1024;
    font->first_char = 32;
    font->size = size;
    s32 stride = font->texture_w * 4;
    ScratchArena scratch = begin_scratch(0);

    // u8 data single channel as alpha
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

    // u32 data 4 channel as rgba
    String8 bitmap_rgba;
    bitmap_rgba.size = (u64)(font->texture_w * font->texture_h * 4);
    bitmap_rgba.str = push_array(scratch.arena, u8, bitmap_rgba.size);
    u32_buffer_from_u8_buffer(&bitmap_a, &bitmap_rgba); // convert from Alpha to RBGA in order to use 1 shader

    D3D11_TEXTURE2D_DESC desc = {
        .Width = (u32)font->texture_w,
        .Height = (u32)font->texture_h,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
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

//static bool
//load_font_ttf(Arena* arena, String8 path, Font* font){
//    File file = os_file_open(path, GENERIC_READ, OPEN_EXISTING);
//    assert_fh(file);
//
//    String8 data =  os_file_read(arena, file);
//    if(!stbtt_InitFont(&font->info, (u8*)data.str, 0)){
//        return(false);
//    }
//
//    os_file_close(file);
//    return(true);
//}

// TODO: change this to load a baked bitmap, and use UV to access correct glyph
//static void
//load_font_glyphs(Arena* arena, Font* font, RGBA color, f32 size){
//    stbtt_GetFontVMetrics(&font->info, &font->ascent, &font->descent, &font->line_gap);
//    font->vertical_offset = font->ascent - font->descent + font->line_gap;
//    font->scale = stbtt_ScaleForPixelHeight(&font->info, size);
//
//    for(s32 c=' '; c<='~'; ++c){
//        Glyph* glyph = font->glyphs + c;
//
//        u8* codepoint_bitmap = stbtt_GetCodepointBitmap(&font->info, 0, font->scale, c, &glyph->w, &glyph->h, &glyph->xoff, &glyph->yoff);
//
//        glyph->bitmap.width = glyph->w;
//        glyph->bitmap.height = glyph->h;
//        glyph->bitmap.stride = glyph->w * 4;
//        glyph->bitmap.base = push_array(arena, u8, (u32)(glyph->w*glyph->h*4));
//
//        // get codepoint info
//        stbtt_GetCodepointHMetrics(&font->info, c, &glyph->advance_width, &glyph->lsb);
//        stbtt_GetCodepointBitmapBox(&font->info, c, font->scale, font->scale, &glyph->x0,&glyph->y0,&glyph->x1,&glyph->y1);
//
//        u8* dest_row = (u8*)glyph->bitmap.base + (glyph->h - 1) * (glyph->w * 4);
//        for(s32 y=0; y < glyph->h; ++y){
//            u32* dest = (u32*)dest_row;
//            for(s32 x=0; x < glyph->w; ++x){
//                u8 alpha = *codepoint_bitmap++;
//                f32 linear_alpha = alpha/255.0f;
//                RGBA new_color = {font->color.r, font->color.g, font->color.b, (f32)alpha};
//
//                *dest++ = ((u32)((alpha) << 24) |
//                          ((u32)(color.r * 255.0f) << 16) |
//                          ((u32)(color.g * 255.0f) <<  8) |
//                          ((u32)(color.b * 255.0f) <<  0));
//            }
//            dest_row -= glyph->w * 4;
//        }
//    }
//}

//static s32
//string_width_in_pixels(String8 str, Font* font){
//    s32 result = 0;
//    for(u64 i=0; i < str.size; ++i){
//        u8* c = str.str + i;
//        Glyph* glyph = font->glyphs + *c;
//        result += glyph->advance_width;
//    }
//    return(round_f32_s32((f32)result * font->scale));
//}

#endif
