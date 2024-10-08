#ifndef ASSET_C
#define ASSET_C


static void
load_assets(Arena* arena, Assets* assets){

    //init_texture_table(arena, &texture_table);

    ScratchArena scratch = begin_scratch();
    String8 build_path = os_application_path(scratch.arena);

    Bitmap bm;
    bm = bitmap_file_read(scratch.arena, build_path, str8_literal("sprites/ship2.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Ship].view, &bm);
    bm = bitmap_file_read(arena, build_path, str8_literal("sprites/circle.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Bullet].view, &bm);
    bm = bitmap_file_read(arena, build_path, str8_literal("sprites/asteroid.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Asteroid].view, &bm);

    bm = bitmap_file_read(arena, build_path, str8_literal("sprites/flame1.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Flame1].view, &bm);
    bm = bitmap_file_read(arena, build_path, str8_literal("sprites/flame2.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Flame2].view, &bm);
    bm = bitmap_file_read(arena, build_path, str8_literal("sprites/flame3.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Flame3].view, &bm);
    bm = bitmap_file_read(arena, build_path, str8_literal("sprites/flame4.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Flame4].view, &bm);
    bm = bitmap_file_read(arena, build_path, str8_literal("sprites/flame5.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Flame5].view, &bm);

    bm = bitmap_file_read(arena, build_path, str8_literal("sprites/explosion1.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Explosion1].view, &bm);
    bm = bitmap_file_read(arena, build_path, str8_literal("sprites/explosion2.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Explosion2].view, &bm);
    bm = bitmap_file_read(arena, build_path, str8_literal("sprites/explosion3.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Explosion3].view, &bm);
    bm = bitmap_file_read(arena, build_path, str8_literal("sprites/explosion4.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Explosion4].view, &bm);
    bm = bitmap_file_read(arena, build_path, str8_literal("sprites/explosion5.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Explosion5].view, &bm);
    bm = bitmap_file_read(arena, build_path, str8_literal("sprites/explosion6.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Explosion6].view, &bm);
    Texture texture = {white_shader_resource};
    assets->textures[TextureAsset_White] = texture;

    assets->waves[WaveAsset_Track1] = wave_file_read(arena, build_path, str8_literal("sounds/track1.wav"));
    assets->waves[WaveAsset_Track2] = wave_file_read(arena, build_path, str8_literal("sounds/track2.wav"));
    assets->waves[WaveAsset_Track3] = wave_file_read(arena, build_path, str8_literal("sounds/track3.wav"));
    assets->waves[WaveAsset_Track4] = wave_file_read(arena, build_path, str8_literal("sounds/track4.wav"));
    assets->waves[WaveAsset_Track5] = wave_file_read(arena, build_path, str8_literal("sounds/track5.wav"));
    assets->waves[WaveAsset_Rail1]  = wave_file_read(arena, build_path, str8_literal("sounds/rail1.wav"));
    assets->waves[WaveAsset_Rail2]  = wave_file_read(arena, build_path, str8_literal("sounds/rail2.wav"));
    assets->waves[WaveAsset_Rail3]  = wave_file_read(arena, build_path, str8_literal("sounds/rail3.wav"));
    assets->waves[WaveAsset_Rail4]  = wave_file_read(arena, build_path, str8_literal("sounds/rail4.wav"));
    assets->waves[WaveAsset_Rail5]  = wave_file_read(arena, build_path, str8_literal("sounds/rail5.wav"));

    assets->waves[WaveAsset_GameWon] = wave_file_read(arena, build_path, str8_literal("sounds/game_won.wav"));
    assets->waves[WaveAsset_GameLost] = wave_file_read(arena, build_path, str8_literal("sounds/game_lost.wav"));
    assets->waves[WaveAsset_AsteroidBreak1] = wave_file_read(arena, build_path, str8_literal("sounds/asteroid_break1.wav"));
    assets->waves[WaveAsset_AsteroidBreak2] = wave_file_read(arena, build_path, str8_literal("sounds/asteroid_break2.wav"));
    assets->waves[WaveAsset_AsteroidBreak3] = wave_file_read(arena, build_path, str8_literal("sounds/asteroid_break3.wav"));
    assets->waves[WaveAsset_ShipExplode] = wave_file_read(arena, build_path, str8_literal("sounds/ship_explode.wav"));
    assets->waves[WaveAsset_Music] = wave_file_read(arena, build_path, str8_literal("sounds/music.wav"));

    assets->fonts[FontAsset_Arial] = font_ttf_read(arena, build_path, str8_literal("fonts/arial.ttf"), 16); // not monospace
    assets->fonts[FontAsset_Golos] = font_ttf_read(arena, build_path, str8_literal("fonts/GolosText-Regular.ttf"), 16);
    assets->fonts[FontAsset_Consolas] = font_ttf_read(arena, build_path, str8_literal("fonts/consola.ttf"), 16); // monospace

    end_scratch(scratch);
}


#endif
