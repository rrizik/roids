#include <mmdeviceapi.h>
#include <audioclient.h>

static IMMDeviceEnumerator* device_enumerator;
static IAudioClient* audio_client;
static IMMDevice* audio_device;
static IAudioRenderClient *render_client;
static IAudioClock* audio_clock;


static WAVEFORMATEX* wave_format;
static WAVEFORMATEX wf;
static u32 buffer_size;
static f32 volume = 0.03f;
static f32 frequency = 0.0f;
//static u32 duration = 10000000;
REFERENCE_TIME duration = 10000000;

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

    // get audio format
    hr = audio_client->GetMixFormat(&wave_format);
    if(FAILED(hr)){
        assert_hr(hr);
        return(hr);
    }

    wf = {0};
    wf.wFormatTag = WAVE_FORMAT_PCM;
    wf.nChannels = 2;
    wf.nSamplesPerSec = 48000;
    wf.wBitsPerSample = 16;
    wf.nBlockAlign = (u16)(wf.nChannels * wf.wBitsPerSample / 8);
    wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
    wf.cbSize = 0;

    //hr = audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, duration, 0, wave_format, 0);
    hr = audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, duration, 0, &wf, 0);
    if(FAILED(hr)){
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

    hr = audio_client->GetService(__uuidof(IAudioClock), (void**)&audio_clock);
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

    u8* buffer;
    u32 available_size = buffer_size - padding;
    hr = render_client->GetBuffer(available_size, &buffer);
    if(FAILED(hr)){
        assert_hr(hr);
        return(hr);
    }

    u64 device_pos;
    hr = audio_clock->GetPosition(&device_pos, 0);
    if(FAILED(hr)){
        assert_hr(hr);
        return(hr);
    }
    u32 play_cursor = (u32)((device_pos + padding) % buffer_size);
    u32 write_cursor = (u32)((device_pos + buffer_size - padding) % buffer_size);

    //print("devicep(%i) - play(%i) - write(%i) - s(%i)\n", device_pos, play_cursor, write_cursor, buffer_size);

    f32 time = 0.0f;
    print("aoskdj\n");
    for(u32 i=0; i < available_size / wf.nChannels; ++i){
        time += 1.0f / (f32)wf.nSamplesPerSec;
        if(time > 1.0f){
            time = 0.0f;
        }
        f32 sine_value = (f32)sin_f32((2.0f * PI_f32 * freq * time));
        //f32 sine_value = (f32)sin_f32((2.0f * PI_f32 * freq * time) / (f32)wave_format->nSamplesPerSec);

        // scale the sine value to the range -0.nf to 0.nf
        sine_value *= volume;

        u32 sample_position = (write_cursor + i * wf.nBlockAlign) % buffer_size;
        u64 write_index = (write_cursor + i) % buffer_size;
        print("wc(%i) - (%i) - (%i)\n", write_cursor, sample_position, write_index);


        f32* buffer_f32 = (f32*)buffer;
        if(wf.nChannels == 2){
            buffer_f32[sample_position] = sine_value;
            buffer_f32[sample_position + 1] = sine_value;
            //buffer_f32[write_index * wf.nChannels] = sine_value;
            //buffer_f32[(write_index * wf.nChannels) + 1] = sine_value;
        }
        else{
            //buffer_f32[i * wf.nChannels] = sine_value;
        }

    }

    hr = render_client->ReleaseBuffer(available_size, 0);
    if(FAILED(hr)){
        assert_hr(hr);
        return(hr);
    }

    return(hr);
}

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
    CoTaskMemFree(wave_format);
    return(hr);
}
