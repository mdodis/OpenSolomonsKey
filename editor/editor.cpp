#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <fstream>
#include <stdio.h>

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include "portable-file-dialogs.h"

#define internal static
#define global   static
#define persist  static
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef int32_t   b32;
#include "osk_math.h"
#include "map/entity.h"
#include "resources.h"

global unsigned g_width = 1100;
global unsigned g_height = 755;
global int g_mouse_x = -1;
global int g_mouse_y = -1;

enum Mode {
    Block,
    Player,
    Enemy,
    Door,
    Key,
    Pickup,
    Hidden,
};
global int g_mode = Mode::Block;

global struct {
    
    struct {
        int to_place = ET_BlockFrail;
    } block;
    
    struct {
        ivec2 last_player_pos = ivec2{-1, -1};
        bool looking_left = true;
    } player;
    
    struct {
        ivec2 last_door_pos = ivec2{-1, -1};
    } door;
    
    Entity sel = {ET_BlockFrail, {0}};
} tool;

inline fvec2 sv2(const sf::Vector2f& v) {
    return fvec2{v.x, v.y};
}

inline sf::Vector2f sv2(const fvec2 &v) {
    return sf::Vector2f(v.x, v.y);
}

internal void custom_param_checkbox_u64(const char *prompt, u64 *destination, u64 truthy, u64 falsy) {
    bool result = (*destination) == truthy;
    ImGui::Checkbox(prompt, &result);
    *destination = (result) ? truthy : falsy;
}

internal void custom_param_slider_double(const char *prompt, double *destination, float min, float max) {
    float result = float(*destination);
    ImGui::SliderFloat(prompt, &result, (float)min, (float)max, "%.1f");
    *destination = result;
}

internal void draw_set_enemy_parameters(Entity *entity) {
    switch(get_enemy_type(entity)) {
        case MT_Goblin: {
            custom_param_checkbox_u64("Looking Left?", &entity->params[1].as_u64, 0, 1);
            custom_param_slider_double("Speed", &entity->params[2].as_f64, 80.0f, 200.0f);
        } break;
        
        case MT_Ghost: {
            custom_param_checkbox_u64("Looking Left?", &entity->params[1].as_u64, 0, 1);
            custom_param_slider_double("Speed", &entity->params[2].as_f64, 100.0f, 300.0f);
        }break;
    }
}

#define TILEMAP_ROWS (12)
#define TILEMAP_COLS (15)
sf::Texture entities[ET_Count];
sf::Texture enemies[MT_Count];
sf::Texture pickups[PT_Count];

global Entity tilemap[TILEMAP_COLS][TILEMAP_ROWS];

inline internal Entity &get_tile_entity(ivec2 pos) {
    return tilemap[pos.x][pos.y];
}

internal void clear_tile(ivec2 pos) {
    Entity &entity = get_tile_entity(pos);
    entity.type = ET_EmptySpace;
    entity.params[0].as_ptype = PT_Count;
}

internal void click_tile(fvec2 pixel_pos, sf::Mouse::Button btn) {
    ivec2 tile_pos = map_position_to_tile(pixel_pos);
    fvec2 tile_pixel_pos = tile_to_position(tile_pos);
    
    if (tile_pos.x >= 0 && tile_pos.x < TILEMAP_COLS &&
        tile_pos.y >= 0 && tile_pos.y < TILEMAP_ROWS) {
        
        if (btn == sf::Mouse::Button::Left) {
            
            switch (g_mode) {
                
                case Mode::Block: {
                    get_tile_entity(tile_pos).type = tool.sel.type;
                }break;
                
                case Mode::Player: {
                    
                    if (tool.player.last_player_pos.x != -1) {
                        clear_tile(tool.player.last_player_pos);
                    }
                    
                    get_tile_entity(tile_pos).type = tool.sel.type;
                    
                    tool.player.last_player_pos = tile_pos;
                }break;
                
                case Mode::Door: {
                    if (tool.door.last_door_pos.x != -1) {
                        clear_tile(tool.door.last_door_pos);
                    }
                    
                    get_tile_entity(tile_pos).type = tool.sel.type;
                    
                    tool.door.last_door_pos = tile_pos;
                } break;
                
                case Mode::Enemy: {
                    get_tile_entity(tile_pos) = tool.sel;
                } break;
                
                case Mode::Pickup: {
                    get_tile_entity(tile_pos) = tool.sel;
                } break;
                
                case Mode::Hidden: {
                    
                    Entity &entity = get_tile_entity(tile_pos);
                    entity.params[0].as_ptype = tool.sel.params[0].as_ptype;
                    
                } break;
                
                default: break;
            }
        } else if (g_mode == Mode::Hidden) {
            Entity &entity = get_tile_entity(tile_pos);
            entity.params[0].as_ptype = PT_Count;
        } else {
            clear_tile(tile_pos);
        }
    }
    
}

internal sf::Texture *get_entity_texture(Entity *e) {
    switch (e->type) {
        
        case ET_BlockSolid:
        case ET_BlockFrail: {
            return &entities[e->type];
        }break;
    }
    return 0;
}

internal void draw_enemy(sf::RenderTexture &drw, sf::Sprite &bob, Entity *entity, ivec2 tile_pos) {
    int c = tile_pos.x;
    int r = tile_pos.y;
    EnemyType enemy_type = get_enemy_type(entity);
    bob.setTexture(enemies[enemy_type]);
    bob.setPosition(sf::Vector2f(c * 64, r * 64));
    
    switch (enemy_type) {
        case MT_Goblin: {
            
            if (entity->params[1].as_u64 == 1)
                bob.setTextureRect(sf::IntRect(64, 0, -64, 64));
            else
                bob.setTextureRect(sf::IntRect(0, 0, 64, 64));
            
        } break;
        
        case MT_Ghost: {
            if (entity->params[1].as_u64 == 1)
                bob.setTextureRect(sf::IntRect(64, 0, -64, 64));
            else
                bob.setTextureRect(sf::IntRect(0, 0, 64, 64));
            
        }break;
    }
    
    drw.draw(bob);
}

internal void draw_tilemap(sf::RenderTexture &drw) {
    sf::Sprite bob;
    for (int c = 0; c < TILEMAP_COLS; c += 1) {
        for (int r = 0; r < TILEMAP_ROWS; r += 1) {
            ivec2 tile_pos = ivec2{c, r};
            Entity &entity = get_tile_entity(tile_pos);
            EntityType entity_type = entity.type;
            
            switch (entity_type) {
                case ET_EmptySpace:
                case ET_BlockFrail:
                case ET_BlockSolid: {
                    
                    if (entity_type != ET_EmptySpace) {
                        bob.setTexture(entities[entity_type]);
                        bob.setPosition(sf::Vector2f(c * 64, r * 64));
                        drw.draw(bob);
                    }
                    
                    if (entity.params[0].as_ptype < PT_Count && g_mode == Mode::Hidden) {
                        bob.setTexture(pickups[entity.params[0].as_ptype]);
                        bob.setPosition(sf::Vector2f(c * 64, r * 64));
                        bob.setColor(sf::Color(255,255,255,126));
                        drw.draw(bob);
                        bob.setColor(sf::Color(255,255,255,255));
                    }
                    
                }break;
                case ET_Player: {
                    bob.setTexture(entities[entity_type]);
                    bob.setPosition(sf::Vector2f(c * 64, r * 64));
                    
                    if (tool.player.looking_left)
                        bob.setTextureRect(sf::IntRect(0, 0, 64, 64));
                    else
                        bob.setTextureRect(sf::IntRect(64, 0, -64, 64));
                    
                    drw.draw(bob);
                }break;
                
                case ET_Door: {
                    bob.setTexture(entities[entity_type]);
                    bob.setPosition(sf::Vector2f(c * 64, r * 64));
                    drw.draw(bob);
                } break;
                
                case ET_Enemy: {
                    draw_enemy(drw, bob, &entity, tile_pos);
                }break;
                
                case ET_Pickup: {
                    PickupType pickup_type = entity.params[0].as_ptype;
                    bob.setTexture(pickups[pickup_type]);
                    bob.setPosition(sf::Vector2f(c * 64, r * 64));
                    drw.draw(bob);
                }break;
            }
        }
    }
    
}

internal inline sf::IntRect tile_offset(int r, int c) {
    return sf::IntRect(r * 64, c * 64, 64, 64);
}

inline internal void selectable_enemy(const char *name, EnemyType type, EnemyType *result) {
    if (ImGui::Selectable(name, *result == type)) 
        *result = type;
}

inline internal void selectable_pickup(const char *name, PickupType type, PickupType *result) {
    if (ImGui::Selectable(name, *result == type)) 
        *result = type;
}

internal void select_pickups(PickupType *result) {
    if (*result < 0 || *result >= PT_Count) *result = PT_Bag100;
    selectable_pickup("Bag 100", PT_Bag100, result);
    selectable_pickup("Bag 200", PT_Bag200, result);
    selectable_pickup("Bag 500", PT_Bag500, result);
    
    selectable_pickup("Bag 1K", PT_Bag1000, result);
    selectable_pickup("Bag 2K", PT_Bag2000, result);
    selectable_pickup("Bag 5K", PT_Bag5000, result);
    
    selectable_pickup("Bag 10K", PT_Bag10000, result);
    selectable_pickup("Bag 20K", PT_Bag20000, result);
    
    selectable_pickup("Jewel 100", PT_Jewel100, result);
    selectable_pickup("Jewel 200", PT_Jewel200, result);
    selectable_pickup("Jewel 500", PT_Jewel500, result);
    
    selectable_pickup("Jewel 1K", PT_Jewel1000, result);
    selectable_pickup("Jewel 2K", PT_Jewel2000, result);
    selectable_pickup("Jewel 5K", PT_Jewel5000, result);
    
    selectable_pickup("Jewel 10K", PT_Jewel10000, result);
    selectable_pickup("Jewel 20K", PT_Jewel20000, result);
    selectable_pickup("Jewel 50K", PT_Jewel50000, result);
    
    selectable_pickup("Bell", PT_Bell, result);
    selectable_pickup("Destruction Potion", PT_PotionDestruction, result);
    selectable_pickup("Hourglass", PT_Hourglass, result);
    selectable_pickup("Switchable Jewel", PT_JewelChange, result);
    selectable_pickup("Range Jewel", PT_JewelRange, result);
    selectable_pickup("Super Range Jewel", PT_JewelRange2, result);
    selectable_pickup("Fire Potion", PT_PotionFire, result);
    selectable_pickup("Super Fire Potion", PT_PotionSuperFire, result);
    selectable_pickup("Fire Growth Potion", PT_PotionFireGrowth, result);
    selectable_pickup("Life Potion", PT_PotionLife, result);
    selectable_pickup("Time Potion", PT_PotionTime1, result);
    selectable_pickup("Stronger Time Potion", PT_PotionTime2, result);
    selectable_pickup("Seal", PT_SolomonSeal, result);
    selectable_pickup("Sphinx", PT_Sphinx, result);
    selectable_pickup("Paper Cane", PT_PaperCrane, result);
    selectable_pickup("Solomon's Key", PT_SolomonsKey, result);
}

internal void select_enemies(EnemyType *result) {
    
    if (*result < 0 || *result >= MT_Count) *result = MT_Goblin;
    
    selectable_enemy("Goblin", MT_Goblin, &tool.sel.params[0].as_etype);
    selectable_enemy("Ghost", MT_Ghost, &tool.sel.params[0].as_etype);
}

int main() {
	sf::RenderWindow window(sf::VideoMode(g_width, g_height), "Open Solomon's Key Editor");
    
	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	sf::Clock deltaClock;
    
    persist sf::Cursor cursor_default;
    cursor_default.loadFromSystem(sf::Cursor::Type::Arrow);
    
    sf::RectangleShape rect(sf::Vector2f(64.f,64.f));
    
    sf::RenderTexture level_texture;
    sf::Sprite level_texture_sprite;
    level_texture.create(15 * 64, 12 * 64);
    level_texture_sprite.setTexture(level_texture.getTexture());
    //level_texture_sprite.setOrigin(level_texture.getSize().x / 2, level_texture.getSize().y / 2);
    // NOTE(miked): load stuff here
    entities[ET_BlockFrail].loadFromFile("res/essentials.png", tile_offset(0,0));
    entities[ET_BlockSolid].loadFromFile("res/essentials.png", tile_offset(0,1));
    entities[ET_Player].loadFromFile("res/dana_all.png", tile_offset(0,0));
    entities[ET_Door].loadFromFile("res/essentials.png", tile_offset(4,2));
    
    enemies[MT_Gargoyle].loadFromFile("res/gargoyle_all.png", tile_offset(0,2));
    enemies[MT_Wyvern].loadFromFile("res/wyvern_all.png", tile_offset(0,0));
    enemies[MT_Dragon].loadFromFile("res/dragon_all.png", tile_offset(0,0));
    enemies[MT_Goblin].loadFromFile("res/goblin_all.png", tile_offset(0,0));
    enemies[MT_Ghost].loadFromFile("res/ghost_all.png", tile_offset(0,0));
    
    pickups[PT_Bag10000].loadFromFile("res/pickups.png", tile_offset(0,0));
    pickups[PT_Bag20000].loadFromFile("res/pickups.png", tile_offset(1,0));
    pickups[PT_Bag100].loadFromFile("res/pickups.png", tile_offset(2,0));
    pickups[PT_Bag200].loadFromFile("res/pickups.png", tile_offset(3,0));
    pickups[PT_Bag500].loadFromFile("res/pickups.png", tile_offset(4,0));
    pickups[PT_Bag1000].loadFromFile("res/pickups.png", tile_offset(5,0));
    pickups[PT_Bag2000].loadFromFile("res/pickups.png", tile_offset(6,0));
    
    pickups[PT_Bag5000].loadFromFile("res/pickups.png", tile_offset(0,1));
    pickups[PT_Bell].loadFromFile("res/pickups.png", tile_offset(1,1));
    pickups[PT_Jewel20000].loadFromFile("res/pickups.png", tile_offset(2,1));
    pickups[PT_Jewel1000].loadFromFile("res/pickups.png", tile_offset(3,1));
    pickups[PT_Jewel2000].loadFromFile("res/pickups.png", tile_offset(4,1));
    pickups[PT_Jewel10000].loadFromFile("res/pickups.png", tile_offset(5,1));
    pickups[PT_PotionDestruction].loadFromFile("res/pickups.png", tile_offset(6,1));
    
    pickups[PT_Hourglass].loadFromFile("res/pickups.png", tile_offset(0,2));
    pickups[PT_Jewel5000].loadFromFile("res/pickups.png", tile_offset(1,2));
    pickups[PT_Jewel50000].loadFromFile("res/pickups.png", tile_offset(2,2));
    pickups[PT_Jewel100].loadFromFile("res/pickups.png", tile_offset(3,2));
    pickups[PT_Jewel200].loadFromFile("res/pickups.png", tile_offset(4,2));
    pickups[PT_Jewel500].loadFromFile("res/pickups.png", tile_offset(5,2));
    pickups[PT_JewelChange].loadFromFile("res/pickups.png", tile_offset(6,2));
    
    pickups[PT_JewelRange].loadFromFile("res/pickups.png", tile_offset(0,3));
    pickups[PT_JewelRange2].loadFromFile("res/pickups.png", tile_offset(1,3));
    pickups[PT_PotionFire].loadFromFile("res/pickups.png", tile_offset(2,3));
    pickups[PT_PotionSuperFire].loadFromFile("res/pickups.png", tile_offset(3,3));
    pickups[PT_PotionFireGrowth].loadFromFile("res/pickups.png", tile_offset(4,3));
    pickups[PT_PotionLife].loadFromFile("res/pickups.png", tile_offset(5,3));
    pickups[PT_PotionTime1].loadFromFile("res/pickups.png", tile_offset(6,3));
    
    pickups[PT_PotionTime2].loadFromFile("res/pickups.png", tile_offset(0,4));
    pickups[PT_ScrollExtension].loadFromFile("res/pickups.png", tile_offset(1,4));
    pickups[PT_SolomonSeal].loadFromFile("res/pickups.png", tile_offset(2,4));
    pickups[PT_Sphinx].loadFromFile("res/pickups.png", tile_offset(3,4));
    pickups[PT_PaperCrane].loadFromFile("res/pickups.png", tile_offset(4,4));
    pickups[PT_SolomonsKey].loadFromFile("res/pickups.png", tile_offset(5,4));
    
    // NOTE: defaults
    
    level_texture_sprite.setScale(sf::Vector2f(0.775, 0.775));
    level_texture_sprite.setPosition(sf::Vector2f(324, 78));
    
    for (int c = 0; c < TILEMAP_COLS; c += 1) {
        for (int r = 0; r < TILEMAP_ROWS; r += 1) {
            tilemap[c][r].params[0].as_ptype = PT_Count;
        }
    }
    
    tool.sel.params[0].as_ptype = PT_Count;
    
    while (window.isOpen()) {
        sf::Vector2i mouse_delta(0,0);
		sf::Event event;
		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(event);
            
            switch (event.type) {
                case sf::Event::Closed: {
                    window.close();
                }break;
                
                case sf::Event::Resized: {
                    g_width = event.size.width;
                    g_height = event.size.height;
                    // update the view to the new size of the window
                    sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                    window.setView(sf::View(visibleArea));
                }break;
                
                case sf::Event::MouseMoved: {
                    int x = event.mouseMove.x;
                    int y = event.mouseMove.y;
                    
                    if (g_mouse_x >= 0) {
                        int dx = x - g_mouse_x;
                        int dy = y - g_mouse_y;
                        mouse_delta.x = dx;
                        mouse_delta.y = dy;
                        
                        if (!io.WantCaptureMouse && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) {
                            
                            sf::Vector2f pos = level_texture_sprite.getPosition();
                            pos += sf::Vector2f(mouse_delta.x, mouse_delta.y);
                            level_texture_sprite.setPosition(pos);
                        }
                    }
                    g_mouse_x = x;
                    g_mouse_y = y;
                }break;
                
                case sf::Event::MouseWheelScrolled: {
                    
                    if (!io.WantCaptureMouse && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) {
                        auto scale = level_texture_sprite.getScale();
                        scale.x += event.mouseWheelScroll.delta * 0.025f;
                        scale.y += event.mouseWheelScroll.delta * 0.025f;
                        level_texture_sprite.setScale(scale);
                    }
                } break;
                
                case sf::Event::MouseButtonPressed: {
                    sf::Vector2f pos(event.mouseButton.x, event.mouseButton.y);
                    sf::Vector2f scale = level_texture_sprite.getScale();
                    
                    if (!io.WantCaptureMouse && !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) {
                        sf::Vector2f offset;
                        sf::Vector2f sz = sf::Vector2f(level_texture.getSize());
                        sf::Vector2f szo = sf::Vector2f(level_texture.getSize());
                        
                        offset = level_texture_sprite.getPosition();
                        pos -= offset;
                        
                        pos.x *= szo.x / (sz.x * scale.x);
                        pos.y *= szo.y / (sz.y * scale.y);
                        
                        click_tile(sv2(pos), event.mouseButton.button);
                    }
                }break;
            }
            
        }
        
        static ImGuiIO& ioImgui = ImGui::GetIO();
        
        persist bool imguiHasCursorPrev = true;
        
        bool imguiHasCursor = ioImgui.WantCaptureMouse || ioImgui.WantCaptureKeyboard;
        
        if (imguiHasCursor != imguiHasCursorPrev)
        {
            if (imguiHasCursor)
            {
                ioImgui.ConfigFlags &= !ImGuiConfigFlags_NoMouseCursorChange;
            }
            else
            {
                ioImgui.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
                window.setMouseCursor(cursor_default);
            }
            imguiHasCursorPrev = imguiHasCursor;
        }
        
        ImGui::SFML::Update(window, deltaClock.restart());
		
        ImGui::SetNextWindowSize(ImVec2(256, window.getSize().y));
        ImGui::SetNextWindowPos(ImVec2(0,0));
        
        ImGui::Begin("osked");
        ImGui::RadioButton("Block",  &g_mode, (int)Mode::Block ); ImGui::SameLine();
        ImGui::RadioButton("Enemy",  &g_mode, (int)Mode::Enemy ); ImGui::SameLine();
        ImGui::RadioButton("Pickup", &g_mode, (int)Mode::Pickup);
        ImGui::RadioButton("Door",   &g_mode, (int)Mode::Door  ); ImGui::SameLine();
        ImGui::RadioButton("Key",    &g_mode, (int)Mode::Key   ); ImGui::SameLine();
        ImGui::RadioButton("Player", &g_mode, (int)Mode::Player);
        ImGui::RadioButton("Hidden", &g_mode, (int)Mode::Hidden);
        
        switch (g_mode) {
            case Mode::Block: {
                ImGui::TextWrapped("Click to place a block, right click to clear.");
                ImGui::Separator();
                
                if ((tool.sel.type != ET_BlockFrail) && (tool.sel.type != ET_BlockSolid))
                    tool.sel.type = ET_BlockFrail;
                
                if (ImGui::Selectable("Frail", tool.sel.type == ET_BlockFrail)) 
                    tool.sel.type = ET_BlockFrail;
                
                if (ImGui::Selectable("Solid", tool.sel.type == ET_BlockSolid))
                    tool.sel.type = ET_BlockSolid;
                
            }break;
            
            case Mode::Player: {
                ImGui::TextWrapped("Click to place player spawn point");
                ImGui::Separator();
                
                tool.sel.type = ET_Player;
                ImGui::Checkbox("Looking left?", &tool.player.looking_left);
            } break;
            
            case Mode::Door: {
                ImGui::TextWrapped("Click to place door");
                ImGui::Separator();
                
                tool.sel.type = ET_Door;
            }break;
            
            case Mode::Enemy: {
                ImGui::TextWrapped("Select enemy, edit it's parameters, and place it in the tilemap");
                ImGui::Separator();
                
                tool.sel.type = ET_Enemy;
                
                select_enemies(&tool.sel.params[0].as_etype);
                
                draw_set_enemy_parameters(&tool.sel);
            } break;
            
            case Mode::Pickup: {
                ImGui::TextWrapped("Select a pickup and place it on the tilemap");
                ImGui::Separator();
                tool.sel.type = ET_Pickup;
                
                ImGui::BeginChild("Pickups", ImVec2(ImGui::GetWindowWidth() * 0.95f, 256));
                select_pickups(&tool.sel.params[0].as_ptype);
                ImGui::EndChild();
                
            } break;
            
            case Mode::Hidden: {
                // TODO(miked): place enemy behind enemy
                ImGui::TextWrapped("Place a hidden item");
                ImGui::Separator();
                
                ImGui::BeginChild("Pickups", ImVec2(ImGui::GetWindowWidth() * 0.95f, 256));
                select_pickups(&tool.sel.params[0].as_ptype);
                ImGui::EndChild();
                
            }break;
            
            default:break;
        }
        
		ImGui::End();
        
        //ImGui::ShowDemoWindow();
        
        window.clear();
        level_texture.clear();
        for (int c = 0; c < 15; c += 1){
            for (int r = 0; r < 12; r += 1){
                rect.setPosition(sf::Vector2f(c * 64.f, r * 64.f));
                level_texture.draw(rect);
            }
        }
        draw_tilemap(level_texture);
        level_texture.display();
        
        window.draw(level_texture_sprite);
        ImGui::SFML::Render(window);
		window.display();
    }
}