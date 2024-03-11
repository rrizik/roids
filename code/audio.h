#include <mmdeviceapi.h>
#include <audioclient.h>

static IMMDeviceEnumerator* device_enumerator;
static IAudioClient* audio_client;
static IMMDevice* audio_device;
static IAudioRenderClient *render_client;
static WAVEFORMATEX* wave_format;
static WAVEFORMATEX wf;
static u32 buffer_size_in_seconds;
static u32 buffer_size;
static f32 numerator = 0;
static f32 volume = 0.03f;
static f32 frequency = 0.0f;

static HRESULT init_audio(){
    HRESULT hr = S_OK;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&device_enumerator);
    if (FAILED(hr)) {
        assert_hr(hr);
        return(hr);
    }

    // get the default audio device
    hr = device_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &audio_device);
    if (FAILED(hr)) {
        assert_hr(hr);
        return(hr);
    }

    // activate the audio client interface
    hr = audio_device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, (void**)&audio_client);
    if (FAILED(hr)) {
        assert_hr(hr);
        return(hr);
    }

    // get audio format
    hr = audio_client->GetMixFormat(&wave_format);
    if (FAILED(hr)) {
        assert_hr(hr);
        return(hr);
    }
    wf.wFormatTag = WAVE_FORMAT_PCM;

    u32 seconds = 2;
    u32 buffer_size_in_seconds = wave_format->nSamplesPerSec * wave_format->nChannels * seconds;
    //hr = audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 2000000, 0, wave_format, 0);
    hr = audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 2000000, 0, wave_format, 0);
    if (FAILED(hr)) {
        assert_hr(hr);
        return(hr);
    }

    // get the size of the buffer
    hr = audio_client->GetBufferSize(&buffer_size);
    if (FAILED(hr)) {
        assert_hr(hr);
        return(hr);
    }

    // get the render client
    hr = audio_client->GetService(__uuidof(IAudioRenderClient), (void**)&render_client);
    if (FAILED(hr)) {
        assert_hr(hr);
        return(hr);
    }

    // start the audio stream
    hr = audio_client->Start();
    if (FAILED(hr)) {
        assert_hr(hr);
        return(hr);
    }

    return(hr);
}

static HRESULT audio_start(){
    hr = audio_client->Start();
    if (FAILED(hr)) {
        assert_hr(hr);
        return(hr);
    }
    return(hr);
}

static HRESULT audio_stop(){
    hr = audio_client->Stop();
    if (FAILED(hr)){
        assert_hr(hr);
        return(hr);
    }
    return(hr);
}

static HRESULT audio_play(f32 freq){
    HRESULT hr = S_OK;

    u32 padding;
    hr = audio_client->GetCurrentPadding(&padding);
    if (FAILED(hr)) {
        assert_hr(hr);
        return(hr);
    }

    u8* buffer;
    u32 available_size = buffer_size - padding;
    hr = render_client->GetBuffer(available_size, &buffer);
    if (FAILED(hr)) {
        assert_hr(hr);
        return(hr);
    }

    for(u32 i=0; i < available_size; ++i){
        f32 time = (f32)(numerator++ / (f32)buffer_size);
        if((u32)numerator > buffer_size){
            numerator = 0.0f;
        }
        f32 sine_value = (f32)sin_f32((2.0f * PI_f32 * freq * time));
        //f32 sine_value = (f32)sin_f32((2.0f * PI_f32 * freq * time) / (f32)wave_format->nSamplesPerSec);

        // Scale the sine value to the range -0.3f to 0.3f
        sine_value *= volume;

        f32* buffer_f32 = (f32*)buffer;
        buffer_f32[i * wave_format->nChannels] = sine_value;
        buffer_f32[i * wave_format->nChannels + 1] = sine_value;
    }

    hr = render_client->ReleaseBuffer(available_size, 0);
    if (FAILED(hr)) {
        assert_hr(hr);
        return(hr);
    }

    return(hr);
}

static HRESULT audio_release(){
    HRESULT hr = S_OK;

    if(audio_client != 0){
        hr = audio_client->Stop();
        if (FAILED(hr)) {
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
