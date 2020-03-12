#include <string.h>

/*
NOTE: Using wave file format with 48000Hz sample-rate,
2 channels, 16b/sample, anything other than that will break it

NOTE: Thank you casey!
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
#define OSK_WAVE_FORMAT_PCM 0x0001
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


/* Loads a 16bit/sample, 48Khz, 2 channel audio file in interleaved format*/
internal RESSound
Wave_load_from_file(const char* file)
{
    WaveHeader* header;
    RESSound result = {};
    char* data = platform_load_entire_file(file);
    assert(data);
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
                assert(fmt->nchannels == AUDIO_CHANNELS);
                assert(fmt->samples_per_sec == AUDIO_SAMPLERATE);
                inform("Channels       : %d", fmt->nchannels);
                inform("Samples        : %d", fmt->samples_per_sec);
                inform("BPP            : %d", fmt->bits_per_sample);
                inform("Block/channels : %d", (fmt->block_align / fmt->nchannels) * 8);

            }break;
            case WAVEChunkData:
            {
                result.data = Wave_get_chunk(iter);
                result.num_samples = RiffIterator_size(iter);
                inform("NUM Samples    : %d", result.num_samples);
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

internal void audio_play_sound(const RESSound* resound, b32 looping = false, SoundType sound_type = SoundEffect)
{
    if (g_audio.all_sounds_size >= AUDIO_MAX_SOUNDS)
        return;


    Sound new_sound =
    {
        .max_counter = resound->num_samples,
        .looping = looping,
        .resource = resound,
        .type = sound_type
    };

    g_audio.all_sounds[g_audio.all_sounds_size++] = new_sound;

}

internal void audio_toggle_playing(SoundType type)
{
    for (i32 i = 0; i < g_audio.all_sounds_size; ++i)
    {
        Sound* sound_ref = g_audio.all_sounds + i;
        if (sound_ref->type == type)
        {
            sound_ref->playing = !sound_ref->playing;
        }
    }

}

internal void audio_update_all_sounds()
{
    if (g_audio.all_sounds_size == 0) return;

    u32 cached_size = g_audio.all_sounds_size;
    for (i32 i = 0; i < cached_size; ++i)
    {
        Sound* sound_ref = g_audio.all_sounds + i;

        if (sound_ref->counter >= sound_ref->max_counter / 2)
        {
            sound_ref->counter = 0;
            if (!sound_ref->looping)
            {
                sound_ref->resource = 0;
                g_audio.all_sounds_size--;
                if (g_audio.all_sounds_size == 0) return;

                b32 will_loop_inside = false;
                // resize the sound array
                i32 previous = i;
                for (i32 j = i + 1; j < cached_size; j++)
                {
                    if (!will_loop_inside)
                    {
                        draw_text("will_loop_inside", 3);
                        will_loop_inside = true;
                    }

                    Sound sound_copy = g_audio.all_sounds[j];
                    g_audio.all_sounds[previous] = sound_copy;
                    previous++;
                }

            }

        }
    }
}

internal void
audio_update(const InputState* const istate, u64 samples_to_write)
{

    i16* out = (i16*)g_audio.buffer;
    //audio_update_all_sounds();

    memset(g_audio.buffer, 0, AUDIO_BUFFER_SIZE);
    for (u32 i = 0; i < samples_to_write; i += 1)
    {
        i16 sample[AUDIO_CHANNELS] = {};


        for (i32 current_sound_idx = 0;
             current_sound_idx < g_audio.all_sounds_size;
             current_sound_idx++)
        {
            Sound* current_sound = g_audio.all_sounds + current_sound_idx;
            warn_unless(current_sound->resource, "");

            if (!current_sound->playing) continue;

            if (!(current_sound->resource))
            {
#if 1
                // TODO(miked): fixme
                fprintf(stderr, "no sound resource; idx %d sz %d\n",
                        current_sound_idx, g_audio.all_sounds_size);
                puts("exiting...");
                fflush(stdout);
                exit(-1);
#endif
                continue;

            }

            u64 new_sound_counter = current_sound->counter;
            if (current_sound->counter >= current_sound->max_counter / 2)
                continue;


            i32 current_sample[AUDIO_CHANNELS] =
            {
                (i32)sample[0],
                (i32)sample[1]
            };

            i32 sample16[AUDIO_CHANNELS] = {0, 0};

            i16* data = (i16*)current_sound->resource->data;
            sample16[0] = data[new_sound_counter++];
            sample16[1] = data[new_sound_counter++];

            if ((current_sample[0] + sample16[0]) > SHRT_MAX ||
                (current_sample[0] + sample16[0]) < SHRT_MIN ||
                (current_sample[1] + sample16[1]) > SHRT_MAX ||
                (current_sample[1] + sample16[1]) < SHRT_MIN )
            {
                continue;
            }

            sample[0] = current_sample[0] + sample16[0];
            sample[1] = current_sample[1] + sample16[1];
            current_sound->counter = new_sound_counter;
        }


        *out++ += sample[0] * g_audio.volume;
        *out++ += sample[1] * g_audio.volume;
    }

}
