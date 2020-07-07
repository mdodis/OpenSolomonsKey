#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <fstream>

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

global unsigned g_width = 700;
global unsigned g_height = 500;
global int g_mouse_x = -1;
global int g_mouse_y = -1;

enum Mode {
    Block,
    Player,
    Enemy,
    Door,
    Key,
    Pickup
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
    
    Entity sel = {ET_BlockFrail, {0}};
} tool;

inline fvec2 sv2(const sf::Vector2f& v) {
    return fvec2{v.x, v.y};
}

inline sf::Vector2f sv2(const fvec2 &v) {
    return sf::Vector2f(v.x, v.y);
}

#define TILEMAP_ROWS (12)
#define TILEMAP_COLS (15)
global EntityType tilemap[TILEMAP_COLS][TILEMAP_ROWS];

internal void clear_tile(ivec2 pos) {
    tilemap[pos.x][pos.y] = ET_EmptySpace;
}

internal void click_tile(fvec2 pixel_pos, bool leftmb = true) {
    ivec2 tile_pos = map_position_to_tile(pixel_pos);
    fvec2 tile_pixel_pos = tile_to_position(tile_pos);
    
    if (tile_pos.x >= 0 && tile_pos.x < TILEMAP_COLS &&
        tile_pos.y >= 0 && tile_pos.y < TILEMAP_ROWS) {
        
        if (leftmb) {
            
            switch (g_mode) {
                
                case Mode::Block: {
                    tilemap[tile_pos.x][tile_pos.y] = tool.sel.type;
                    
                }break;
                
                case Mode::Player: {
                    
                    if (tool.player.last_player_pos.x != -1) {
                        clear_tile(tool.player.last_player_pos);
                    }
                    
                    tilemap[tile_pos.x][tile_pos.y] = tool.sel.type;
                    
                    tool.player.last_player_pos = tile_pos;
                }break;
                
                default: break;
            }
        } else {
            clear_tile(tile_pos);
        }
    }
    
}

sf::Texture entities[ET_Count];

internal sf::Texture *get_entity_texture(Entity *e) {
    switch (e->type) {
        
        case ET_BlockSolid:
        case ET_BlockFrail: {
            return &entities[e->type];
        }break;
    }
    return 0;
}

internal void draw_tilemap(sf::RenderTexture &drw) {
    sf::Sprite bob;
    for (int c = 0; c < TILEMAP_COLS; c += 1) {
        for (int r = 0; r < TILEMAP_ROWS; r += 1) {
            
            EntityType entity_type = tilemap[c][r];
            
            switch (entity_type) {
                case ET_EmptySpace: break;
                
                case ET_BlockFrail:
                case ET_BlockSolid: {
                    bob.setTexture(entities[entity_type]);
                    bob.setPosition(sf::Vector2f(c * 64, r * 64));
                    drw.draw(bob);
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
            }
        }
    }
    
}

internal inline sf::IntRect tile_offset(int r, int c) {
    return sf::IntRect(r * 64, c * 64, 64, 64);
}

int main() {
	sf::RenderWindow window(sf::VideoMode(g_width, g_height), "Open Solomon's Key Editor");
    
	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	sf::Clock deltaClock;
    
    sf::RectangleShape rect(sf::Vector2f(64.f,64.f));
    
    sf::RenderTexture level_texture;
    sf::Sprite level_texture_sprite;
    level_texture.create(15 * 64, 12 * 64);
    level_texture_sprite.setTexture(level_texture.getTexture());
    
    // NOTE(miked): load stuff here
    entities[ET_BlockFrail].loadFromFile("res/essentials.png", tile_offset(0,0));
    entities[ET_BlockSolid].loadFromFile("res/essentials.png", tile_offset(0,1));
    entities[ET_Player].loadFromFile("res/dana_all.png", tile_offset(0,0));
    
    tilemap[1][1] = ET_BlockSolid;
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
                
                case sf::Event::MouseButtonPressed: {
                    sf::Vector2f pos(event.mouseButton.x, event.mouseButton.y);
                    
                    if (!io.WantCaptureMouse && !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) {
                        pos -= level_texture_sprite.getPosition();
                        click_tile(sv2(pos), event.mouseButton.button == sf::Mouse::Left);
                    }
                }break;
            }
            
        }
        ImGui::SFML::Update(window, deltaClock.restart());
		
        ImGui::Begin("osked");
        
        ImGui::RadioButton("Block",  &g_mode, (int)Mode::Block ); ImGui::SameLine();
        ImGui::RadioButton("Player", &g_mode, (int)Mode::Player); ImGui::SameLine();
        ImGui::RadioButton("Enemy",  &g_mode, (int)Mode::Enemy ); ImGui::SameLine();
        ImGui::RadioButton("Door",   &g_mode, (int)Mode::Door  ); ImGui::SameLine();
        ImGui::RadioButton("Key",    &g_mode, (int)Mode::Key   ); ImGui::SameLine();
        ImGui::RadioButton("Pickup", &g_mode, (int)Mode::Pickup);
        
        switch (g_mode) {
            case Mode::Block: {
                ImGui::Text("Click to place a block, right click to clear.");
                
                if (tool.sel.type != ET_BlockFrail || tool.sel.type != ET_BlockSolid)
                    tool.sel.type = ET_BlockFrail;
                
                if (ImGui::Selectable("Frail", tool.sel.type == ET_BlockFrail)) 
                    tool.sel.type = ET_BlockFrail;
                
                if (ImGui::Selectable("Solid", tool.sel.type == ET_BlockSolid)) tool.sel.type = ET_BlockSolid;
                
            }break;
            
            case Mode::Player: {
                ImGui::Text("Click to place player spawn point");
                
                tool.sel.type = ET_Player;
                ImGui::Checkbox("Looking left?", &tool.player.looking_left);
            } break;
            
            default:break;
        }
        
		ImGui::End();
        
        ImGui::ShowDemoWindow();
        
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