#ifndef WASAPI_CPP
#define WASAPI_CPP

// todo: change all asserts to error logs
static HRESULT
audio_init(u16 channels, u32 samples_per_sec, u16 bits_per_sample){
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
    device_enumerator->Release();

    // activate the audio client interface
    hr = audio_device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, (void**)&audio_client);
    if(FAILED(hr)){
        assert_hr(hr);
        return(hr);
    }
    audio_device->Release();

    wave_format = {0};
    wave_format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    wave_format.nChannels = channels;
    wave_format.nSamplesPerSec = samples_per_sec;
    wave_format.wBitsPerSample = bits_per_sample;
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

static HRESULT audio_play_sine(f32 freq){
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
        sine_value *= 0.03f;

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

static bool
audio_play(WaveAsset id, f32 volume, bool loop){
    Wave wave = tm->assets.wave[id];
    WaveCursor cursor = {0};
    cursor.id = id;
    cursor.at = 0;
    cursor.volume = volume;
    cursor.loop = loop;

    if(wave_cursors_count < WAVE_CURSORS_COUNT_MAX){
        wave_cursors[wave_cursors_count++] = cursor;
        return(true);
    }
    return(false);
}

static HRESULT audio_play_cursors(){
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


    memset(buffer, 0, available_size * wave_format.nBlockAlign);
    f32* buffer_f32 = (f32*)buffer;
    for(s32 cursor_i=0; cursor_i < (s32)wave_cursors_count; ++cursor_i){
        WaveCursor* cursor = wave_cursors + cursor_i;
        Wave* wave = tm->assets.wave + cursor->id;

        u32 wave_remainder = wave->size - cursor->at;
        u32 iter_size = wave_remainder > available_size ? available_size : wave_remainder;

        if(cursor->at < wave->size){
            for(s32 i=0; i < iter_size; ++i){
                f32 sample = ((s16)(wave->base[(cursor->at + i)])) * (1.0f / 32768.0f);

                buffer_f32[ i * wave_format.nChannels]      += sample * cursor->volume;
                buffer_f32[(i * wave_format.nChannels) + 1] += sample * cursor->volume;
            }

        }

        cursor->at += iter_size;
        if(cursor->loop){
            if(cursor->at >= wave->size){
                cursor->at = 0;
            }
        }
    }

    hr = render_client->ReleaseBuffer(available_size, 0);
    if(FAILED(hr)){
        assert_hr(hr);
        return(hr);
    }

    return(hr);
}
// todo: next step -
// mix them into the buffer

static void
audio_release(){
    if(audio_client != 0){
        audio_client->Release();
    }

    render_client->Release();
}

#endif
