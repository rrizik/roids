#ifndef WASAPI_H
#define WASAPI_H

#pragma comment(lib, "ole32.lib")
#include <mmdeviceapi.h>
#include <audioclient.h>

typedef struct WaveCursor{
    u16* base;
    u32 sample_count; // size in samples, not bytes
    bool active;
    u32 index;

    u32 at;
    f32 volume;
    bool loop;
} WaveCursor;

#define WAVE_CURSORS_MAX 32
global WaveCursor wave_cursors[WAVE_CURSORS_MAX];
global s32  wave_cursors_count;
u32 free_cursors[WAVE_CURSORS_MAX];
u32 free_cursors_at;

static void
cursors_clear(void){
    free_cursors_at = WAVE_CURSORS_MAX - 1;
    for(u32 i = free_cursors_at; i <= free_cursors_at; --i){
        WaveCursor* cursor = wave_cursors + i;
        cursor->active = false;
        free_cursors[i] = free_cursors_at - i;
    }
    wave_cursors_count = 0;
}

static void
remove_cursor(WaveCursor* cursor){
    free_cursors[++free_cursors_at] = cursor->index;
    wave_cursors_count--;
    *cursor = {0};
}

static WaveCursor*
get_cursor(){
    if(free_cursors_at < WAVE_CURSORS_MAX){
        u32 free_cursor_index = free_cursors[free_cursors_at--];
        WaveCursor* cursor = wave_cursors + free_cursor_index;
        cursor->index = free_cursor_index;
        wave_cursors_count++;
        cursor->active = true;

        return(cursor);
    }
    return(0);
}

static IMMDeviceEnumerator* device_enumerator;
static IMMDevice*           audio_device;
static IAudioClient*        audio_client;
static IAudioRenderClient*  render_client;
static WAVEFORMATEX         wave_format;

static u32 buffer_samples;
REFERENCE_TIME default_device_period;
REFERENCE_TIME minimum_device_period;

static void init_wasapi(u16 channels, u32 samples_per_sec, u16 bits_per_sample);
static void wasapi_start(void);
static void wasapi_stop(void);
//static HRESULT audio_play_sine(f32 freq);
static void wasapi_play_cursors(void);
static bool wasapi_play(Wave* wave, f32 volume, bool loop);
static void wasapi_release(void);

#endif

