#ifndef ASSET_H
#define ASSET_H

typedef enum FontAsset{
    FontAsset_Arial,
    FontAsset_Golos,
    FontAsset_Consolas,

    FontAsset_Count,
} FontAsset;

typedef enum WaveAsset{
    WaveAsset_Track1,
    WaveAsset_Track2,
    WaveAsset_Track3,
    WaveAsset_Track4,
    WaveAsset_Track5,
    WaveAsset_Rail1,
    WaveAsset_Rail2,
    WaveAsset_Rail3,
    WaveAsset_Rail4,
    WaveAsset_Rail5,

    WaveAsset_Count,
} WaveAsset;

typedef enum TextureAsset{
    TextureAsset_Ship,
    TextureAsset_Bullet,
    TextureAsset_Asteroid,

    TextureAsset_Flame1,
    TextureAsset_Flame2,
    TextureAsset_Flame3,
    TextureAsset_Flame4,
    TextureAsset_Flame5,

    TextureAsset_Explosion1,
    TextureAsset_Explosion2,
    TextureAsset_Explosion3,
    TextureAsset_Explosion4,
    TextureAsset_Explosion5,
    TextureAsset_Explosion6,

    TextureAsset_White,

    TextureAsset_Count,
} TextureAsset;
#endif
