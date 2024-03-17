#pragma comment(lib, "ole32")
#include <mmdeviceapi.h>
#include <audioclient.h>

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

typedef struct ChunkFmt{
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
} ChunkFmt;

typedef struct Wave{
    u16 audio_format;
    u16 channels;
    u32 samples_per_sec;
    u32 bytes_per_sec;
    u16 block_align;
    u16 bits_per_sample;

    u8* base;
    u32 at;
    u32 size;
} Wave;

static IMMDeviceEnumerator* device_enumerator;
static IAudioClient* audio_client;
static IMMDevice* audio_device;
static IAudioRenderClient *render_client;
static WAVEFORMATEX wave_format;

static u32 buffer_size;
static u32 numerator = 0;
static f32 volume = 0.03f;
REFERENCE_TIME duration = 10000000;

static Wave
load_wave(Arena* arena, String8 filename){
    Wave result = {0};

    ScratchArena scratch = begin_scratch(0);
    String8 full_path = str8_concatenate(scratch.arena, path_data, filename);
    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    assert_h(file.handle);

    String8 data = os_file_read(scratch.arena, file);
    WaveHeader* header = (WaveHeader*)data.str;

    u8* chunks = data.str + sizeof(WaveHeader);
    u32 chunk_inc = 0;
    bool data_found = false;
    while(!data_found){
        ChunkInfo* c = (ChunkInfo*)(chunks + chunk_inc);
        if(str8_compare(str8(c->chunk_id, 4), chunk_ids[ChunkId_RIFF])){
        }
        if(str8_compare(str8(c->chunk_id, 4), chunk_ids[ChunkId_WAVE])){
        }
        if(str8_compare(str8(c->chunk_id, 3), chunk_ids[ChunkId_FMT])){
            ChunkFmt* c_fmt = (ChunkFmt*)((u8*)c + sizeof(ChunkInfo));

            result.audio_format = c_fmt->audio_format;
            result.channels = c_fmt->channels;
            result.samples_per_sec = c_fmt->samples_per_sec;
            result.bytes_per_sec = c_fmt->bytes_per_sec;
            result.block_align = c_fmt->block_align;
            result.bits_per_sample = c_fmt->bits_per_sample;
        }
        if(str8_compare(str8(c->chunk_id, 4), chunk_ids[ChunkId_DATA])){
            result.size = c->chunk_size;
            result.base = (u8*)c + sizeof(ChunkInfo);
            data_found = true;
        }
        chunk_inc += c->chunk_size + sizeof(ChunkInfo);
    }

    return(result);
}

static HRESULT init_audio(){
    HRESULT hr = S_OK;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&device_enumerator);
    if(FAILED(hr)){
        assert_hr(hr);
        return(hr);
    }

    // get the default audio device
    hr = device_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &audio_device);
    if(FAILED(hr)){
        assert_hr(hr);
        return(hr);
    }

    // activate the audio client interface
    hr = audio_device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, (void**)&audio_client);
    if(FAILED(hr)){
        assert_hr(hr);
        return(hr);
    }

    wave_format = {0};
    wave_format.wFormatTag = 1;
    //wave_format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    wave_format.nChannels = 2;
    wave_format.nSamplesPerSec = 48000;
    wave_format.wBitsPerSample = 24;
    wave_format.nBlockAlign = (u16)(wave_format.nChannels * wave_format.wBitsPerSample / 8);
    wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
    wave_format.cbSize = 0;

    hr = audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, duration, 0, &wave_format, 0);
    if (FAILED(hr)) {
        assert_hr(hr);
        return(hr);
    }

    // get the size of the buffer
    hr = audio_client->GetBufferSize(&buffer_size);
    if(FAILED(hr)){
        assert_hr(hr);
        return(hr);
    }

    // get the render client
    hr = audio_client->GetService(__uuidof(IAudioRenderClient), (void**)&render_client);
    if(FAILED(hr)){
        assert_hr(hr);
        return(hr);
    }

    // start the audio stream
    hr = audio_client->Start();
    if(FAILED(hr)){
        assert_hr(hr);
        return(hr);
    }

    return(hr);
}

static HRESULT audio_start(){
    hr = audio_client->Start();
    if(FAILED(hr)){
        assert_hr(hr);
        return(hr);
    }
    return(hr);
}

static HRESULT audio_stop(){
    hr = audio_client->Stop();
    if(FAILED(hr)){
        assert_hr(hr);
        return(hr);
    }
    return(hr);
}

static HRESULT audio_play(f32 freq){
    HRESULT hr = S_OK;

    u32 padding;
    hr = audio_client->GetCurrentPadding(&padding);
    if(FAILED(hr)){
        assert_hr(hr);
        return(hr);
    }
    u32 available_size = buffer_size - padding;

    u8* buffer;
    hr = render_client->GetBuffer(available_size, &buffer);
    if(FAILED(hr)){
        assert_hr(hr);
        return(hr);
    }

    static f32 time;
    for(u32 i=0; i < available_size; ++i){
        time = (f32)((f32)(numerator++ % buffer_size) / (f32)buffer_size);

        f32 sine_value = sin_f32((2.0f * PI_f32 * freq * time));

        // scale the sine value to the range -0.nf to 0.nf
        sine_value *= volume;

        f32* buffer_f32 = (f32*)buffer;
        if(wave_format.nChannels == 2){
            buffer_f32[i * wave_format.nChannels] = sine_value;
            buffer_f32[(i * wave_format.nChannels) + 1] = sine_value;
        }
        else{
            buffer_f32[i * wave_format.nChannels] = sine_value;
        }
    }

    hr = render_client->ReleaseBuffer(available_size, 0);
    if (FAILED(hr)) {
        assert_hr(hr);
        return(hr);
    }

    return(hr);
}

static HRESULT audio_play_wave(Wave* wave){
    HRESULT hr = S_OK;

    u32 padding;
    hr = audio_client->GetCurrentPadding(&padding);
    if (FAILED(hr)) {
        assert_hr(hr);
        return(hr);
    }
    u32 available_size = buffer_size - padding;

    u8* buffer;
    hr = render_client->GetBuffer(available_size, &buffer);
    if (FAILED(hr)) {
        assert_hr(hr);
        return(hr);
    }


    u32 wave_remainder = wave->size - wave->at;
    u32 total_bytes = available_size * wave->channels * (wave->bits_per_sample / 8);
    if (wave_remainder >= total_bytes){

        memory_copy(buffer, wave->base + wave->at, total_bytes);

        wave->at += total_bytes;

        if (wave->at >= wave->size){
            wave->at = 0;
        }
    }
    else {
        memory_copy(buffer, wave->base + wave->at, wave_remainder);
        memset((u8*)buffer + wave_remainder, 0, total_bytes - wave_remainder);
        wave->at = 0;
    }

    hr = render_client->ReleaseBuffer(available_size, 0);
    if(FAILED(hr)){
        assert_hr(hr);
        return(hr);
    }

    return(hr);
}
// todo: next step -
// deserialize sound from a file - looking wav fileformat leading it
// loop over samples in that file - question later
// mix them into the buffer

static HRESULT audio_release(){
    HRESULT hr = S_OK;

    if(audio_client != 0){
        hr = audio_client->Stop();
        if(FAILED(hr)){
            assert_hr(hr);
            return(hr);
        }
        audio_client->Release();
    }

    device_enumerator->Release();
    audio_device->Release();
    render_client->Release();
    //CoTaskMemFree(wave_format);
    return(hr);
}
