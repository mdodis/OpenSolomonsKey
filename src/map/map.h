/* date = May 13th 2020 9:57 pm */
#ifndef MAP_H
#define MAP_H

struct MapEntity {
};

////////////////////////////////
// put your entity handling code here

// add a normal (non-enemy, non-pickup) entity
bool add_tilemap_entity(EntityType type, int row, int col) {
    
}

// add a pickup that's hidden in row,col
bool add_tilemap_hidden_entity(PickupType type, int row, int col) {
}

////////////////////////////////
static char* _load_entire_file(const char* path) {
    size_t size;
    char* data;
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    
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

static char* string_nextline(char* c) {
    while (*c != '\n') c++;
    return c + 1;
}

static char* string_parse_uint(char *c, unsigned long *out_i) {
#define IS_DIGIT(x) (x >= '0' && x <= '9')
    unsigned long res = 0;
    while (*c && IS_DIGIT(*c)) {
        res *= 10;
        res += *c - '0';
        c++;
    }
    *out_i = res;
    return c;
#undef IS_DIGIT
}

static char* eat_whitepspace(char *c) {
    while(*c == ' ' || *c == '\t') c++;
    return c;
}

char *parse_tilemap_hidden(char *c, int row, int col) {
    c = eat_whitepspace(c);
    while(*c && *c == ',') {
        unsigned long eid;
        c++;
        c = string_parse_uint(c, &eid);
        add_tilemap_hidden_entity(PickupType(eid), row, col);
    }
}

static char *parse_long(char *c, long *d) {
    char *end;
    *d = strtol(c, &end, 10);
    return end;
}


static char *Ghost_custom(char *c) {
    c = parse_custom_double(ghost, c, 1, 200.f);
    c = parse_custom_double(ghost, c, 1, 200.f);
    return c;
}

static char *parse_enemy(char *c, int row, int col);
static char *parse_enemy_type(char *c, EnemyType *type);
static char *parse_enemy_custom(char *c, EnemyType type, int row, int col);

static char *parse_enemy(char *c, int row, int col) {
    EnemyType type;
    c = parse_enemy_type(c, &type);
    c = parse_enemy_custom(enemy, c, type, sprite_initial_pos);
    
    return c;
}

static char *parse_enemy_type(char *c, EnemyType *type) {
    long tl;
    assert(*c == ',');
    c++;
    c = parse_long(c, &tl);
    *type = (EnemyType)tl;
    return c;
}

static char *parse_enemy_custom(char *c, EnemyType type, int row, int col) {
    switch(type) {
        case EnemyType::Goblin: {
            c = Goblin_custom(s, c);
        }break;
        
        case EnemyType::Ghost: {
            c = Ghost_custom(s, c);
        }break;
        
        case EnemyType::BlueFlame: {
            c = BlueFlame_custom(s, c);
        }break;
        
        case EnemyType::KMirror: {
            c = KMirror_custom(s, c, sprite_initial_pos);
        }break;
    }
    return c;
}

internal bool load_map_from_file(const char *filename, int *errcode) {
    
    bool level_validity[] = {
        false,  // Door exists
        false,  // Key exists
        false   // Player exists
    };
    
    const int TILEMAP_COLS = 15;
    const int TILEMAP_ROWS = 12;
    
    char *str = _load_entire_file(filename);
    char *c = str;
    
    if (!str) {
        if (errcode) *errcode = 333;
        return false;
    }
    
    while (*c) {
        
        switch(*c) {
            case '#': {
                c = string_nextline(c);
            } break;
            
            case '0':case '1':case '2':
            case '3':case '4':case '5':
            case '6':case '7':case '8':
            case '9': {
                unsigned long res;
                c = string_parse_uint(c, &res);
                
                if (res >= ET_Count) {
                    // error: "Entity type %lld does not exist in loader!"
                    return false;
                }
                
                if (counter_x >= TILEMAP_COLS) {
                    counter_x = 0;
                    counter_y++;
                }
                
                if (is_valid_tilemap_object((EntityBaseType) res)) {
                    add_tilemap_entity(res, counter_y, counter_x);
                    c = parse_tilemap_hidden(c, counter_y, counter_x);
                } else {
                    
                    if (res == ET_Door) {
                        level_validity[0] = true;
                        add_tilemap_entity(ET_Door, counter_y, counter_x);
                    } else if (res == eKey) {
                        level_validity[1] = true;
                        add_tilemap_entity(ET_Key, counter_y, counter_x);
                    }
                    
                    switch((EntityType)res) {
                        
                        case ET_PlayerSpawnPoint:{
                            level_validity[2] = true;
                            add_tilemap_entity(ET_PlayerSpawnPoint, counter_y, counter_x);
                        }break;
                        
                        case eEnemy: {
                            c = parse_enemy(c, counter_y, counter_x);
                        }break;
                        
                        case ePickup: {
                            if (*c == ',') {
                                c++;
                                long type;
                                c = parse_long(c, &type);
                                if (pickup_type_is_valid((PickupType)type)) {
                                    add_tilemap_pickup(PickupType type, counter_y, counter_x);
                                }
                            } else {
                                add_tilemap_pickup(PT_SolomonsKey, counter_y, counter_x);
                            }
                            
                        }break;
                        
                        default:{
                            error("sprite type %Iu not available for make_", res);
                            exit(0);
                        }break;
                    }
                    
                }
                
                counter_x++;
                
            } break;
            
            
            default: {
                if (c[0] == 'B' && c[1] == 'G' && c[2] == ' ') {
                    c += 3;
                    long bg;
                    c = parse_long(c, &bg);
                    inform("Using background %d", bg);
                }
                c++;
            } break;
        }
    }
}

#endif //MAP_H
