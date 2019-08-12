
#define DEFAULT_AUDIO_SAMPLERATE 44100
#define DEFAULT_AUDIO_CHANNELS 1
#define FRAMES_PER_BUFFER 1024
#define BUFFER_SIZE FRAMES_PER_BUFFER * DEFAULT_AUDIO_CHANNELS * 2

global u32 g_audio_samplerate = DEFAULT_AUDIO_SAMPLERATE;
PaStream *stream;
i16* buffer[BUFFER_SIZE];

internal void
audio_init()
{
    PaError err;
    err = Pa_Initialize();
    assert(err == paNoError);
    
    err = Pa_OpenDefaultStream(
        &stream,
        0,          /* no input channels */
        DEFAULT_AUDIO_CHANNELS,
        paInt16,  /* 32 bit floating point output */
        DEFAULT_AUDIO_SAMPLERATE,
        FRAMES_PER_BUFFER,        /* frames per buffer, i.e. the number
        of sample frames that PortAudio will
        request from the callback. Many apps
        may want to use
        paFramesPerBufferUnspecified, which
        tells PortAudio to pick the best,
        possibly changing, buffer size.*/
        0, /* this is your callback function */
        0);
    assert(err == paNoError);
    err = Pa_StartStream( stream );
    assert(err == paNoError);
}

internal void 
audio_update()
{    
    // Update audio
    i64 frames_to_write = Pa_GetStreamWriteAvailable(stream);
    if (frames_to_write == 0 ) return;
    else if (frames_to_write < 0)
    {
        puts("audio error");
        exit(-11);
    }
    
    u32 wave_period = DEFAULT_AUDIO_SAMPLERATE / 440;
    
    persist u32 counter = wave_period;
    persist float time = 0.f;
    i16* wav = (i16*)buffer;
    for (u32 i = 0; i < frames_to_write; i += 1)
    {
        if (!counter) counter = wave_period;
        i16 sample = sinf(time) * 800;
        *wav++ = sample;
        counter--;
        
        time += 2.f * M_PI *  1.f / (float)wave_period;
    }
    
    PaError err = Pa_WriteStream(
        stream,
        buffer,
        frames_to_write);
    if ( err != paNoError )
    {
        exit(-1);
    }
}