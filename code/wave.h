#ifndef WAVE_H
#define WAVE_H

typedef enum ChunkId{
    ChunkId_RIFF,
    ChunkId_WAVE,
    ChunkId_FMT,
    ChunkId_DATA,
    ChunkId_COUNT,
} ChunkId;

String8 chunk_ids[ChunkId_COUNT] = {
    str8_literal("RIFF"),
    str8_literal("WAVE"),
    str8_literal("fmt"),
    str8_literal("data"),
};

typedef struct WaveHeader{
    u8 chunk_id[4];
    u32 chunk_size;
    u8 format[4];
} WaveHeader;

typedef struct ChunkInfo{
    u8 chunk_id[4];
    u32 chunk_size;
} ChunkInfo;

typedef struct WaveFormat{
    u16 audio_format;
    u16 channels;
    u32 samples_per_sec;
    u32 bytes_per_sec;
    u16 block_align;
    u16 bits_per_sample;
    u16 ext_size;
    u16 valid_bits_per_sample;
    u32 channel_mask;
    u32 sub_format[4];
} WaveFormat;

typedef struct Wave{
    WaveFormat format;
    u16* base;
    u32 size;
} Wave;

static Wave load_wave(Arena* arena, String8 filename){
    Wave result = {0};

    ScratchArena scratch = begin_scratch(0);
    String8 full_path = str8_concatenate(scratch.arena, build_path, filename);
    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    assert_h(file.handle);

    String8 data = os_file_read(scratch.arena, file);
    WaveHeader* header = (WaveHeader*)data.str;

    u8* chunks = data.str + sizeof(WaveHeader);
    u32 chunk_inc = 0;
    bool data_found = false;
    bool fmt_found = false;

    while(!data_found || !fmt_found){
        ChunkInfo* c = (ChunkInfo*)(chunks + chunk_inc);
        if(str8_compare(str8(c->chunk_id, 4), chunk_ids[ChunkId_RIFF])){
        }
        else if(str8_compare(str8(c->chunk_id, 4), chunk_ids[ChunkId_WAVE])){
        }
        else if(str8_compare(str8(c->chunk_id, 3), chunk_ids[ChunkId_FMT])){
            WaveFormat* format = (WaveFormat*)((u8*)c + sizeof(ChunkInfo));
            result.format = *format;
            fmt_found = true;
        }
        else if(str8_compare(str8(c->chunk_id, 4), chunk_ids[ChunkId_DATA])){
            result.size = c->chunk_size / result.format.block_align; // convert size from bytes to samples
            result.base = (u16*)((u8*)c + sizeof(ChunkInfo));
            data_found = true;
        }
        else if((u8*)c > (data.str + data.size)){
            // todo: log error - failed to load wafvff file
            print("Error: Failed to load WAVE file\n");
            result = {0};
            os_file_close(file);
            return(result);
        }
        chunk_inc += c->chunk_size + sizeof(ChunkInfo);
    }
    os_file_close(file);

    return(result);
}

#endif
