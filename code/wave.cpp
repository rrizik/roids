#ifndef WAVE_C
#define WAVE_C

// todo: samples to bytes - bytes to samples. Maybe
static Wave
wave_file_read(Arena* arena, String8 dir, String8 filename){
    Wave result = {0};

    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_concatenate(scratch.arena, dir, filename);
    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    assert_h(file.handle);

    String8 data = os_file_read(scratch.arena, file);
    WaveHeader* header = (WaveHeader*)data.str;

    // todo: change this to maybe be str8 so that you can use str8 functions
    u8* chunks = data.str + sizeof(WaveHeader);
    u32 chunk_inc = 0;
    bool data_found = false;
    bool fmt_found = false;

    // todo: improve this somewhat. check for RIFF and WAVE, otherwise something is wrong.
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
            result.sample_count = c->chunk_size / result.format.block_align; // convert size from bytes to samples
            result.base = push_array(arena, u16, c->chunk_size);
            memcpy(result.base, (u16*)((u8*)c + sizeof(ChunkInfo)), c->chunk_size);
            data_found = true;
        }
        // todo: this doesn't really make sense I don't think
        else if((u8*)c > (data.str + data.size)){
            // todo: log error - failed to load wafvff file
            print("Error: Failed to load WAVE file\n");
            result = {0};
            os_file_close(file);
            return(result);
        }
        chunk_inc += c->chunk_size + sizeof(ChunkInfo);
    }
    end_scratch(scratch);
    os_file_close(file);

    return(result);
}


#endif
