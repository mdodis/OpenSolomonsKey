
#define IS_DIGIT(x) (x >= '0' && x <= '9')

internal b32
string_cmp_indentifier(
const char* c,
const char* str,
u64* out_size)
{
    u64 size = 0;
    
    while (*c != ' ' && *c != '\n')
    {
        if (*c != *str)
        {
            if (*str == 0) break;
            return false;
        }
        
        size++;
        c++; str++;
    }
    *out_size = size;
    return true;
}

internal const char*
string_trim(const char* c)
{
    while(*c == ' ' || *c == '\n') c++;
    return c;
}

internal const char*
string_parse_uint(const char* c, u64* out_i)
{
    u64 res = 0;
    while (*c && IS_DIGIT(*c))
    {
        res *= 10;
        res += *c - '0';
        
        c++;
    }
    *out_i = res;
    return c;
}

global PaletteEntry g_palette[64];
global u64 g_tilemap[15][12];
global u64 g_palette_size;

internal const char*
parse_pallete_entry(const char* c, PaletteEntryType* out_entry)
{
    u64 stride = 0;
    PaletteEntryType result = PENTRY_UNKNOWN;
    
    for(u32 i = 0; i < PENTRY_UNKNOWN; ++i)
    {
        const char* entry_name = g_all_entries[i];
        if (string_cmp_indentifier(c, entry_name, &stride))
        {
            result = (PaletteEntryType)i;
            break;
        }
        
    }
    
    *out_entry = result;
    return c + stride;
}

internal char*
platform_load_entire_file(const char* path)
{
    u64 size;
    char* data;
    FILE* f = fopen(path, "rb");
    if (!f) exit(-1);
    
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    data = (char*)malloc(size + 1);
    assert(data);
    fread(data, size, 1, f);
    data[size] = 0;
    fclose(f);
    
    return data;
}

internal void load_test_level(
PaletteEntry palette[64],
u64* out_size)
{ 
    char* level = platform_load_entire_file("test_level_2.oskmap");
#if 0
    u64 size;
    FILE* f = fopen("test_level_2.oskmap" , "rb");
    if (!f) exit(-1);
    
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    level = (char*)malloc(size + 1);
    assert(level);
    fread(level, size, 1, f);
    level[size] = 0;
    fclose(f);
#endif
    
    const char* c = level;
    
    u64 palette_size, counter = 0;
    
    while(*c != 0)
    {
        switch(*c)
        {
            case '#':
            {
                c++;
                u64 stride = 0;
                // Check for entries
                PaletteEntryType type;
                const char* result = parse_pallete_entry(c, &type);
                
                if (type != PENTRY_UNKNOWN)
                {
                    palette[counter].type = type;
                    counter++;
                    printf("%s\n", g_all_entries[type]);
                    c = result;
                    continue;
                }
                
                if (string_cmp_indentifier(c, "PALETTE", &stride))
                {
                    puts("PALETTE");
                    c += stride;
                    
                    c = string_trim(c);
                    c = string_parse_uint(c, &palette_size);
                    
                    *out_size = palette_size;
                    
                    printf("%ld\n", (int)palette_size);
                }
                else if (string_cmp_indentifier(c, "MAP", &stride))
                {
                    puts("MAP");
                    c += stride;
                    u64 layer_num;
                    
                    c = string_trim(c);
                    c = string_parse_uint(c, &layer_num);
                    printf("%d\n",(int) layer_num);
                    if (layer_num == 1)
                    {
                        break;
                    }
                    c = string_trim(c);
                    
                    for(u32 j = 0; j < 12; ++j)
                    {
                        for(u32 i = 0; i < 15; ++i)
                        {
                            u64 idx;
                            c = string_trim(c);
                            c = string_parse_uint(c, &idx);
                            
                            g_tilemap[i][j] = idx;
                            
                            printf("%d ", (int)idx);
                        }
                        puts("");
                    }
                    
                }
                else
                {
                    puts("UNKNOWN directive!");
                    exit(6);
                }
                
                if (counter > palette_size)
                {
                    puts("ERR: palette_size smaller than the counter");
                    exit(5);
                }
                
            } break;
            
            default:
            {
                c++;
            } break;
        }
        
    }
}
