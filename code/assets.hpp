#ifndef ASSET_H
#define ASSET_H

typedef enum FontAsset{
    FontAsset_None,
    FontAsset_Arial,
    FontAsset_Golos,
    FontAsset_Consolas,

    FontAsset_Count,
} FontAsset;

typedef enum WaveAsset{
    WaveAsset_None,
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
    WaveAsset_GameWon,
    WaveAsset_GameLost,
    WaveAsset_AsteroidBreak1,
    WaveAsset_AsteroidBreak2,
    WaveAsset_AsteroidBreak3,
    WaveAsset_ShipExplode,
    WaveAsset_Music,

    WaveAsset_Count,
} WaveAsset;

static u64
hash_from_key(String8 key){
    u64 result = 5381;

    for(u64 i=0; i < key.count; ++i){
        result = ((result << 5) + result) + key.data[i];
    }

    return(result);
}

typedef struct TextureHashNode{
    TextureHashNode* next;
    u64 hash;
    String8 key;
    Texture value;
} TextureHashNode;

typedef struct TextureHashTable{
    Arena* arena;
    u64 count;
    TextureHashNode** slots;
} TextureHashTable;
global TextureHashTable texture_table;

static void
init_texture_table(Arena* arena, TextureHashTable* table){
    table->arena = arena;
    table->count = TABLE_DEFAULT_COUNT;
    table->slots = push_array(table->arena, TextureHashNode*, table->count);
}

static void
texture_table_insert(TextureHashTable* table, String8 key, Texture texture){
    // if new table, allocate default amount
    if(table->count == 0){
        table->count = TABLE_DEFAULT_COUNT;
        table->slots = push_array(table->arena, TextureHashNode*, table->count);
    }

    u64 hash = hash_from_key(key);
    u64 slot_idx = hash % table->count;
    TextureHashNode* found_node = 0;

    TextureHashNode* n = table->slots[slot_idx];
    while(n != 0){
        if(n->hash == hash && str8_compare(n->key, key)){
            found_node = n;
            break;
        }
        n = n->next;
    }

    // overwrite found node.
    if(found_node != 0){
        found_node->value = texture;
    }

    // node not found. create new one.
    if(found_node == 0){
        TextureHashNode* node = push_struct(table->arena, TextureHashNode);
        node->value = texture;
        node->hash = hash;
        node->key = key;
        node->next = table->slots[slot_idx];
        table->slots[slot_idx] = node;
    }
}

static Texture
texture_table_lookup(TextureHashTable* table, String8 key){
    u64 hash = hash_from_key(key);
    u64 slot_idx = hash % table->count;
    TextureHashNode* found_node = 0;

    TextureHashNode* n = table->slots[slot_idx];
    while(n != 0){
        if(n->hash == hash && str8_compare(n->key, key)){
            found_node = n;
            break;
        }
        n = n->next;
    }

    Texture result = {0};
    if(found_node != 0){
        result = found_node->value;
    }
    return(result);
}

typedef enum TextureAsset{
    TextureAsset_None,
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

typedef struct Assets{
    Wave    waves[WaveAsset_Count];
    Font    fonts[FontAsset_Count];
    Texture textures[TextureAsset_Count];
} Assets;
static void load_assets(Arena* arena, Assets* assets);

#endif
