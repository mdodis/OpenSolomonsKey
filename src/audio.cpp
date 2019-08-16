
#pragma pack(push, 1)
struct WaveHeader
{
    u32 id;     // "RIFF"
    u32 size;   // 4 + n
    u32 waveid; // "WAVE"
};

struct WaveFmt
{
#define WAVE_FORMAT_PCM 0x0001
    u16 format_tag;
    u16 nchannels;
    u32 samples_per_sec;
    u32 avg_bytes_per_sec;
    u16 block_align;
    u16 bits_per_sample;
    u16 ext_size;
    u16 valid_bits_per_sample;
    u32 channel_mask;
    u8 subformat[16];
};

struct WaveChunk
{
    u32 id;
    u32 size;
};
#pragma pack(pop)


#define RIFF_CODE(a,b,c,d) (u32)((a << 0) | (b << 8) | (c << 16) | (d << 24))

enum WaveChunkType
{
    WAVEChunkFmt  = RIFF_CODE('f','m','t',' '),
    WAVEChunkRiff = RIFF_CODE('R','I','F','F'),
    WAVEChunkData = RIFF_CODE('d','a','t','a'),
    WAVEChunkWave = RIFF_CODE('W','A','V','E'),
};


struct RiffIterator
{
    u8* at;
};

internal RiffIterator
Wave_parse_chunk(void* at)
{
    RiffIterator result;
    result.at = (u8*)at;
    return result;
}

internal RiffIterator
Wave_next_chunk(RiffIterator iter)
{
    WaveChunk* chunk = (WaveChunk*)iter.at;
    u32 size = (chunk->size + 1) & ~1;
    iter.at += chunk->size + sizeof(WaveChunk);
    
    return iter;
}

internal void*
Wave_get_chunk(RiffIterator iter)
{
    return iter.at + sizeof(WaveChunk);
}

internal WaveChunkType
RiffIterator_type(RiffIterator iter)
{
    WaveChunk* chunk = (WaveChunk*)iter.at;
    return (WaveChunkType)chunk->id;
}


internal u32
RiffIterator_size(RiffIterator iter)
{
    WaveChunk* chunk = (WaveChunk*)iter.at;
    return (WaveChunkType)chunk->size;
}


struct Sound
{
    u32 samplerate;
    u32 num_channels;
    u32 num_samples;
    void* data;
};

internal Sound
Wave_load_from_file(const char* file)
{
    WaveHeader* header;
    Sound result = {};
    char* data = platform_load_entire_file(file);
    
    header = (WaveHeader*)data;
    assert(header->id == WAVEChunkRiff && header->waveid == WAVEChunkWave);
    
    
    for (RiffIterator iter = Wave_parse_chunk(header + 1);
         *(u8*)iter.at != 0;
         iter = Wave_next_chunk(iter))
    {
        switch(RiffIterator_type(iter))
        {
            case WAVEChunkFmt:
            {
                WaveFmt* fmt = (WaveFmt*)Wave_get_chunk(iter);
                assert(fmt->format_tag == 1);
                assert((fmt->block_align / fmt->nchannels) == 2);
                printf("Channels       : %d\n", fmt->nchannels);
                printf("Samples        : %d\n", fmt->samples_per_sec);
                printf("BPP            : %d\n", fmt->bits_per_sample);
                printf("Block/channels : %d\n", (fmt->block_align / fmt->nchannels) * 8);
                
            }break;
            case WAVEChunkData:
            {
                result.data = Wave_get_chunk(iter);
                result.num_samples = RiffIterator_size(iter);
                printf("NUM Samples    : %d\n", result.num_samples);
            }break;
            
            default:
            {
                puts("shit");
                assert(0);
            } break;
        }
    }
    return result;
}



#define DEFAULT_AUDIO_SAMPLERATE 48000
#define DEFAULT_AUDIO_CHANNELS 2
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
audio_generate_sine(
u32 tone,
float* time,
u32 intensity, 
void* buffer, 
u32 frames_to_write)
{
    u32 wave_period = DEFAULT_AUDIO_SAMPLERATE / 440;
    
    i16* wav = (i16*)buffer;
    for (u32 i = 0; i < frames_to_write; i += 1)
    {
        i16 sample = sinf(*time) * 800;
        
        for (u32 j = 0; j < DEFAULT_AUDIO_CHANNELS; ++j)
            *wav++ = sample;
        
        *time += 2.f * M_PI *  1.f / (float)wave_period;
    }
    
}

global Sound test_sound;

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
    
    persist float time = 0.f;
    //audio_generate_sine(440,&time,800, buffer, frames_to_write);
    
    
    memset(buffer, 0, BUFFER_SIZE);
    i16* out = (i16*)buffer;
    i16* data = (i16*)test_sound.data;
    b32 noplay = false;
    persist int counter = 0;
    
    for (u32 i = 0; i < frames_to_write; i += 1)
    {
        if (counter >= test_sound.num_samples / 2)
        {
            counter = 0;
            //*out++ = 0;
            //*out++ = 0;
        }
        
        
        *out++ = data[counter++];
        *out++ = data[counter++];
        
        
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
