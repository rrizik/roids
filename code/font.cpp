#ifndef FONT_C
#define FONT_C

static bool load_font_ttf(Arena* arena, String8 path, Font* font, f32 size){
    // open file
    File file = os_file_open(path, GENERIC_READ, OPEN_EXISTING);
    assert_fh(file); // todo: replace all these asserts with if checks so it doesn't explode

    // init font
    String8 file_data =  os_file_read(arena, file);
    if(!stbtt_InitFont(&font->info, (u8*)file_data.str, 0)){
        return(false);
    }
    font->scale = stbtt_ScaleForPixelHeight(&font->info, size);
    stbtt_GetFontVMetrics(&font->info, &font->ascent, &font->descent, &font->line_gap);
    font->vertical_offset = font->ascent - font->descent + font->line_gap;

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

    hr = d3d_device->CreateTexture2D(&desc, &shader_data, &font->atlas.tex);
    assert_hr(hr);
    hr = d3d_device->CreateShaderResourceView(font->atlas.tex, 0, &font->atlas.view);
    assert_hr(hr);

    os_file_close(file);
    end_scratch(scratch);
    return(true);
}

static f32
font_char_width(Font font, u8 c){
    f32 result = 0;
    s32 advance_width, lsb;
    stbtt_GetCodepointHMetrics(&font.info, c, &advance_width, &lsb);
    result = (f32)advance_width * font.scale;
    return(result);
}

static f32
font_string_width(Font font, String8 str){
    f32 result = 0;
    s32 advance_width, lsb;
    for(s32 i=0; i < str.size; ++i){
        u8 c = str.str[i];
        stbtt_GetCodepointHMetrics(&font.info, c, &advance_width, &lsb);
        result += (f32)advance_width * font.scale;
    }
    return(result);
}

#endif
