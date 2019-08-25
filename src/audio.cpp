#include <string.h>
/*
NOTE: Using wave file format with 48000Hz sample-rate,
2 channels, 16b/sample, anything other than that will break it
*/
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


#define DEFAULT_AUDIO_SAMPLERATE 48000
#define DEFAULT_AUDIO_CHANNELS 2
#define FRAMES_PER_BUFFER 1024
#define BUFFER_SIZE FRAMES_PER_BUFFER * DEFAULT_AUDIO_CHANNELS * 2

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


struct RESSound
{
    u32 samplerate;
    u32 num_channels;
    u32 num_samples;
    void* data;
    
};

/* Loads a 16bit/sample, 48Khz, 2 channel audio file in interleaved format*/
internal RESSound
Wave_load_from_file(const char* file)
{
    WaveHeader* header;
    RESSound result = {};
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
                assert(fmt->nchannels == DEFAULT_AUDIO_CHANNELS);
                assert(fmt->samples_per_sec == DEFAULT_AUDIO_SAMPLERATE);
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

global u32 g_audio_samplerate = DEFAULT_AUDIO_SAMPLERATE;
PaStream *stream;
i16* buffer[BUFFER_SIZE];
float g_audio_master_volume = .1f;

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
        i16 sample = sinf(*time) * intensity;
        
        for (u32 j = 0; j < DEFAULT_AUDIO_CHANNELS; ++j)
            *wav++ += sample;
        
        *time += 2.f * M_PI *  1.f / (float)wave_period;
    }
    
}

struct Sound
{
    u64 counter = 0;
    u64 max_counter = ~(0u);
    b32 looping = false;
    b32 playing = false;
    const RESSound* resource = 0;
};

global struct
{
    
#define AUDIO_MAX_SOUNDS 32
    Sound all_sounds[AUDIO_MAX_SOUNDS];
    i32 all_sounds_size = 0;
} g_audio;

internal void audio_play_sound(const RESSound* resound)
{
    warn_unless(
        g_audio.all_sounds_size < AUDIO_MAX_SOUNDS, 
        "Attempted to play more than AUDIO_MAX_SOUNDS");
    if (g_audio.all_sounds_size > 0) return;
    
    for (i32 i = 0; i < g_audio.all_sounds_size; ++i)
    {
        if (g_audio.all_sounds[i].resource == resound) break;
    }
    
    
    Sound new_sound = {
        .max_counter = resound->num_samples,
        .looping = false, 
        .resource = resound};
    
    g_audio.all_sounds[g_audio.all_sounds_size++] = new_sound;
}

internal void audio_update_all_sounds()
{
    for (i32 i = 0; i < AUDIO_MAX_SOUNDS; ++i)
    {
        Sound* sound_ref = g_audio.all_sounds + i;
        
        if (sound_ref->counter >= sound_ref->max_counter / 2)
        {
            sound_ref->counter = 0;
            if (!sound_ref->looping)
            {
                sound_ref->resource = 0;
                g_audio.all_sounds_size--;
                // resize the sound array
                i32 previous = i;
                for (i32 j = i + 1; j < g_audio.all_sounds_size; j++)
                {
                    Sound sound_copy = g_audio.all_sounds[j];
                    g_audio.all_sounds[previous] = sound_copy;
                    previous++;
                }
            }
        }
    }
}

internal void 
audio_update(const InputState* const istate)
{    
    // Update audio
    i64 frames_to_write = Pa_GetStreamWriteAvailable(stream);
    if (frames_to_write == 0 ) return;
    else if (frames_to_write < 0)
    {
        puts("audio error");
        exit(-11);
    }
    memset(buffer, 0, frames_to_write * 2 * 2);
    
    i16* out = (i16*)buffer;
    audio_update_all_sounds();
    
    //printf("num sounds: %d\n", g_audio.all_sounds_size);
    for (u32 i = 0; i < frames_to_write; i += 1)
    {
        i16 sample[DEFAULT_AUDIO_CHANNELS] = {};
        
        
        for (i32 current_sound_idx = 0;
             current_sound_idx < g_audio.all_sounds_size;
             current_sound_idx++)
        {
            Sound* current_sound = g_audio.all_sounds + current_sound_idx;
            
            if (current_sound->counter >= current_sound->max_counter / 2)
            {
                break;
            }
            
            i32 current_sample[DEFAULT_AUDIO_CHANNELS] = {
                sample[0],
                sample[1]};
            
            u64 new_sound_counter = current_sound->counter;
            i32 sample16[DEFAULT_AUDIO_CHANNELS] = {};
            
            i16* data = (i16*)current_sound->resource->data;
            sample16[0] = data[new_sound_counter++];
            sample16[1] = data[new_sound_counter++];
            
            if ((current_sample[0] + sample16[0]) > SHRT_MAX ||
                (current_sample[0] + sample16[0]) < SHRT_MIN ||
                (current_sample[1] + sample16[1]) > SHRT_MAX ||
                (current_sample[1] + sample16[1]) < SHRT_MIN )
            {
                break;
            }
            
            sample[0] = current_sample[0] + sample16[0];
            sample[1] = current_sample[1] + sample16[1];
            current_sound->counter = new_sound_counter;
        }
        
        
        *out++ += sample[0];
        *out++ += sample[1];
        
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
