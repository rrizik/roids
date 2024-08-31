#ifndef FONT_C
#define FONT_C

static Font
font_ttf_read(Arena* arena, String8 dir, String8 filename, f32 size){
    // open file

    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_concatenate(scratch.arena, dir, filename);
    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    end_scratch(scratch);
    assert_h(file.handle);

    // init font
    Font result = {0};
    String8 file_data =  os_file_read(arena, file); // note: stb ttf fonts need to remain loaded in memory.
    if(!stbtt_InitFont(&result.info, (u8*)file_data.str, 0)){
        result.succeed = false;
        return(result);
    }
    result.scale = stbtt_ScaleForPixelHeight(&result.info, size);
    stbtt_GetFontVMetrics(&result.info, &result.ascent, &result.descent, &result.line_gap);

    result.vertical_offset = round_f32((f32)(result.ascent - result.descent + result.line_gap) * result.scale); // note: I don't see value in keeping this in unscaled.

    result.texture_w = 1024;
    result.texture_h = 1024;
    result.first_char = 32;
    result.size = size;

    // u8 data single channel as alpha
    String8 bitmap_a;
    bitmap_a.size = (u64)(result.texture_w * result.texture_h);
    bitmap_a.str = push_array(scratch.arena, u8, bitmap_a.size);

    stbtt_pack_context context;
    if (!stbtt_PackBegin(&context, bitmap_a.str, result.texture_w, result.texture_h, 0, 1, 0)) {
        result.succeed = false;
        return(result);
    }

    stbtt_pack_range range;
    range.chardata_for_range = result.packed_chars;
    range.array_of_unicode_codepoints = NULL; // Indicates that we are using the range
    range.first_unicode_codepoint_in_range = 32;
    range.num_chars = 95;
    range.font_size = size;

    if (!stbtt_PackFontRanges(&context, (u8*)file_data.str, 0, &range, 1)) {
        result.succeed = false;
        return(result);
    }
    stbtt_PackEnd(&context);

    // u32 data 4 channel as rgba
    // warning: incomplete: todo: we do this to avoide having to create anther shader for fonts. This is a huge waste of space and instead we should create the additional shader and keep it single byte
    String8 bitmap_rgba;
    bitmap_rgba.size = (u64)(result.texture_w * result.texture_h * 4);
    bitmap_rgba.str = push_array(scratch.arena, u8, bitmap_rgba.size);

    // note: convert 1 channel to 4 channel.
    // todo: get rid of this when you create a text specific shader, this is a waste
    u32* base_rgba = (u32*)bitmap_rgba.str;
    u8* base_a = (u8*)bitmap_a.str;
    for(s32 i=0; i < bitmap_a.size; ++i){
        *base_rgba = (u32)(*base_a << 24 |
                               255 << 16 |
                               255 << 8  |
                               255 << 0);
        base_rgba++;
        base_a++;
    }

    D3D11_TEXTURE2D_DESC desc = {
        .Width = (u32)result.texture_w,
        .Height = (u32)result.texture_h,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
        .SampleDesc = {1, 0},
        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_SHADER_RESOURCE,
    };

    D3D11_SUBRESOURCE_DATA shader_data = {
        .pSysMem = bitmap_rgba.str,
        .SysMemPitch = (u32)(result.texture_w * 4),
    };
    ID3D11Texture2D* texture;
    hr = d3d_device->CreateTexture2D(&desc, &shader_data, &texture);
    assert_hr(hr);
    hr = d3d_device->CreateShaderResourceView(texture, 0, &result.texture.view);
    assert_hr(hr);

    texture->Release();
    os_file_close(file);
    end_scratch(scratch);
    return(result);
}

// todo: all these functions need to take in Font*
static f32
font_char_width(Font* font, u8 c){
    f32 result = 0;

    s32 advance_width, lsb;
    stbtt_GetCodepointHMetrics(&font->info, c, &advance_width, &lsb);
    result = (f32)advance_width * font->scale;
    return(result);
}

static f32
font_string_width(Font* font, String8 str){
    f32 result = 0;

    s32 advance_width, lsb;
    for(s32 i=0; i < str.size; ++i){
        u8 c = str.str[i];
        stbtt_GetCodepointHMetrics(&font->info, c, &advance_width, &lsb);
        result += (f32)advance_width * font->scale;
    }
    return(result);
}

static f32
font_vertical_offset(Font* font){
    return(font->vertical_offset);
}

static f32
font_ascent(Font* font){
    f32 result = round_f32((f32)font->ascent * font->scale);
    return(result);
}

static f32
font_descent(Font* font){
    f32 result = round_f32((f32)font->descent * font->scale);
    return(result);
}

#endif
