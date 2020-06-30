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

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef int32_t   b32;
#include "map/entity.h"

static unsigned g_width = 700;
static unsigned g_height = 500;
static int g_mouse_x = -1;
static int g_mouse_y = -1;

int main() {
	sf::RenderWindow window(sf::VideoMode(g_width, g_height), "Open Solomon's Key Editor");
    
	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	sf::Clock deltaClock;
    
    sf::RectangleShape rect(sf::Vector2f(64.f,64.f));
    sf::RectangleShape rrect(sf::Vector2f(64.f,64.f));
    rrect.setFillColor(sf::Color::Red);
    
    sf::RenderTexture level_texture;
    sf::Sprite level_texture_sprite;
    level_texture.create(15 * 64, 12 * 64);
    level_texture_sprite.setTexture(level_texture.getTexture());
    
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
                        
                        if (!io.WantCaptureMouse && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
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
                    
                    if (!io.WantCaptureMouse) {
                        pos -= level_texture_sprite.getPosition();
                        rrect.setPosition(pos);
                    }
                }break;
            }
            
        }
        ImGui::SFML::Update(window, deltaClock.restart());
		
        ImGui::Begin("Heyo");
        ImGui::Text("Delta %d %d", mouse_delta.x, mouse_delta.y);
		ImGui::End();
        
        window.clear();
        level_texture.clear();
        for (int c = 0; c < 15; c += 1){
            for (int r = 0; r < 12; r += 1){
                rect.setPosition(sf::Vector2f(c * 64.f, r * 64.f));
                level_texture.draw(rect);
            }
        }
        level_texture.draw(rrect);
        level_texture.display();
        
        window.draw(level_texture_sprite);
        ImGui::SFML::Render(window);
		window.display();
    }
}