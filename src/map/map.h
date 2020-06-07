/* date = May 13th 2020 9:57 pm */
#ifndef MAP_H
#define MAP_H

#include "entity.h"

////////////////////////////////
// put your entity handling code here

// add a normal (non-enemy, non-pickup) entity
bool add_tilemap_entity(EntityType type, int row, int col, void *custom1 = 0);
// add a pickup that's hidden in row,col
bool add_tilemap_hidden_entity(PickupType type, int row, int col);
// add a normal pickup
bool add_tilemap_pickup(PickupType type, int row, int col);
// add an enemy (kmirror is set if the enemy is a parameter for the kmirror)
bool add_tilemap_enemy(EnemyType type, int row, int col, void *param1, void *param2, bool kmirror);
// add background
bool add_tilemap_background(long num);

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
    return c;
}

static char *parse_long(char *c, long *d) {
    char *end;
    *d = strtol(c, &end, 10);
    return end;
}

internal char *parse_double(char *c, double *d){
    char *end;
    *d = strtod(c, &end);
    return end;
}

static char *parse_custom_double(double *ret, char *c, double default_val) {
    *ret = default_val;
    
    if (*c == ',') {
        c++;
        c = parse_double(c, ret);
    }
    return c;
}

static char *parse_custom_long(long *ret, char *c, long default_val) {
    *ret = default_val;
    
    if (*c == ',') {
        c++;
        c = parse_long(c, ret);
    }
    return c;
}

static char *Dana_custom(char *c, int row, int col) {
    long dir;
    c = parse_custom_long(&dir, c, 0);
    add_tilemap_entity(ET_PlayerSpawnPoint, row, col, (void*)&dir);
    return c;
}

static char *Key_custom(char *c, int row, int col) {
    long type;
    c = parse_custom_long(&type, c, 0);
    add_tilemap_entity(ET_Key, row, col, (void*)&type);
    return c;
}

static char *parse_enemy(char *c, int row, int col);
static char *parse_enemy_type(char *c, EnemyType *type);
static char *parse_enemy_custom(char *c, EnemyType type, int row, int col, bool kmirror);
static char *parse_kmirror_enemy(char *c, int index);

static char *Goblin_custom(char *c, int row, int col, bool kmirror) {
    double speed;
    long dir;
    c = parse_custom_double(&speed, c, 80.f);
    c = parse_custom_long(&dir, c, 0);
    add_tilemap_enemy(MT_Goblin, row, col, (void*)&speed, (void*)&dir, kmirror);
    return c;
}

static char *Ghost_custom(char *c, int row, int col, bool kmirror) {
    double speed;
    long dir;
    c = parse_custom_double(&speed, c, 200.f);
    c = parse_custom_long(&dir, c, 0);
    add_tilemap_enemy(MT_Ghost, row, col, (void*)&speed, (void*)&dir, kmirror);
    return c;
}

static char *BlueFlame_custom(char *c, int row, int col, bool kmirror) {
    double time;
    long ignore;
    c = parse_custom_double(&time, c, 1.f);
    c = parse_custom_long(&ignore, c, 0);
    add_tilemap_enemy(MT_BlueFlame, row, col, (void*)&time, (void*)0, kmirror);
    return c;
}

static char *DemonHead_custom(char *c, int row, int col, bool kmirror) {
    double speed;
    long dir;
    c = parse_custom_double(&speed, c, 1.f);
    c = parse_custom_long(&dir, c, 0);
    add_tilemap_enemy(MT_Demonhead, row, col, (void*)&speed, (void*)&dir, kmirror);
    return c;
}

static char *Wyvern_custom(char *c, int row, int col, bool kmirror) {
    double speed;
    long dir;
    c = parse_custom_double(&speed, c, 200.f);
    c = parse_custom_long(&dir, c, 0);
    add_tilemap_enemy(MT_Wyvern, row, col, (void*)&speed, (void*)&dir, kmirror);
    return c;
}

static char *Dragon_custom(char *c, int row, int col, bool kmirror) {
    double speed;
    long dir;
    c = parse_custom_double(&speed, c, 100.f);
    c = parse_custom_long(&dir, c, 0);
    add_tilemap_enemy(MT_Dragon, row, col, (void*)&speed, (void*)&dir, kmirror);
    return c;
}

static char *SparkBall_custom(char *c, int row, int col, bool kmirror) {
    double speed;
    long dir;
    c = parse_custom_double(&speed, c, 100.f);
    c = parse_custom_long(&dir, c, 0);
    add_tilemap_enemy(MT_SparkBall, row, col, (void*)&speed, (void*)&dir, kmirror);
    return c;
}

static char *Gargoyle_custom(char *c, int row, int col, bool kmirror) {
    double speed;
    long dir;
    c = parse_custom_double(&speed, c, 100.f);
    c = parse_custom_long(&dir, c, 0);
    add_tilemap_enemy(MT_Gargoyle, row, col, (void*)&speed, (void*)&dir, kmirror);
    return c;
}

static char *PanelMonster_custom(char *c, int row, int col) {
    double interval;
    long dir;
    c = parse_custom_double(&interval, c, 2.0f);
    c = parse_custom_long(&dir, c, 0);
    add_tilemap_enemy(MT_PanelMonster, row, col, (void*)&interval, (void*)&dir, false);
    return c;
}

static char *parse_kmirror_enemy(char *c, int row, int col) {
    if (*c == ',') {
        c++;
        assert(*c == '{');
        c++;
        
        EnemyType type;
        {
            long tl;
            c = parse_long(c, &tl);
            type = (EnemyType)tl;
            assert(type < MT_Count);
        }
        c = parse_enemy_custom(c, type, row, col, true);
        assert(*c == '}');
        c++;
    }
    return c;
}

static char *KMirror_custom(char *c, int row, int col) {
    double delay, interval;
    
    c = parse_custom_double(&delay, c, 1.f);
    c = parse_custom_double(&interval, c, 1.f);
    add_tilemap_enemy(MT_KMirror, row, col, (void*)&delay, (void*)&interval, false);
    
    c = parse_kmirror_enemy(c, row, col);
    c = parse_kmirror_enemy(c, row, col);
    return c;
}

static char *parse_enemy(char *c, int row, int col) {
    EnemyType type;
    c = parse_enemy_type(c, &type);
    c = parse_enemy_custom(c, type, row, col, false);
    
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

static char *parse_enemy_custom(char *c, EnemyType type, int row, int col, bool kmirror) {
    switch(type) {
        case MT_Goblin: {
            c = Goblin_custom(c, row, col, kmirror);
        }break;
        
        case MT_Ghost: {
            c = Ghost_custom(c, row, col, kmirror);
        }break;
        
        case MT_BlueFlame: {
            c = BlueFlame_custom(c, row, col, kmirror);
        }break;
        
        case MT_KMirror: {
            c = KMirror_custom(c, row, col);
        }break;
        
        case MT_Demonhead: {
            c = DemonHead_custom(c, row, col, kmirror);
        }break;
        
        case MT_Wyvern: {
            c = Wyvern_custom(c, row, col, kmirror);
        }break;
        
        case MT_Dragon: {
            c = Dragon_custom(c, row, col, kmirror);
        }break;
        
        case MT_PanelMonster: {
            c = PanelMonster_custom(c, row, col);
        }break;
        
        case MT_SparkBall: {
            c = SparkBall_custom(c, row, col, kmirror);
        }break;
        
        case MT_Gargoyle: {
            c = Gargoyle_custom(c, row, col, kmirror);
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
    
    const int columns  = 15;
    const int rows  = 12;
    int counter_x, counter_y;
    
    char *str = _load_entire_file(filename);
    char *c = str;
    
    if (!str) {
        if (errcode) *errcode = 333;
        return false;
    }
    counter_y = counter_x = 0;
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
                
                if (counter_x >= columns) {
                    counter_x = 0;
                    counter_y++;
                }
                
                if (is_valid_tilemap_object((EntityType) res)) {
                    add_tilemap_entity((EntityType)res, counter_y, counter_x);
                    c = parse_tilemap_hidden(c, counter_y, counter_x);
                } else {
                    
                    if (res == ET_Door) {
                        level_validity[0] = true;
                        add_tilemap_entity(ET_Door, counter_y, counter_x);
                    } else if (res == ET_Key) {
                        level_validity[1] = true;
                        
                        c = Key_custom(c, counter_y, counter_x);
                    }
                    
                    switch((EntityType)res) {
                        
                        case ET_Door:
                        case ET_Key:{
                            // nop
                        }break;
                        
                        case ET_PlayerSpawnPoint:{
                            level_validity[2] = true;
                            c = Dana_custom(c, counter_y, counter_x);
                            
                        }break;
                        
                        case ET_Enemy: {
                            c = parse_enemy(c, counter_y, counter_x);
                        }break;
                        
                        case ET_Pickup: {
                            if (*c == ',') {
                                c++;
                                long type;
                                c = parse_long(c, &type);
                                if (pickup_type_is_valid((PickupType)type)) {
                                    add_tilemap_pickup((PickupType) type, counter_y, counter_x);
                                }
                            } else {
                                add_tilemap_pickup(PT_SolomonsKey, counter_y, counter_x);
                            }
                            
                        }break;
                        
                        default:{
                            // error("sprite type %Iu not available for make_", res);
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
                    //inform("Using background %d", bg);
                    add_tilemap_background(bg);
                }
                c++;
            } break;
        }
    }
    return true;
}

#endif //MAP_H
