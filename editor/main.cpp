#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <fstream>

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include "portable-file-dialogs.h"
#include "globals.cpp"
#include "func.cpp"

int main()
{
	sf::RenderWindow window(sf::VideoMode(1250, 700), "Open Solomon's Key Editor", sf::Style::Close|sf::Style::Resize);
	
	window.setPosition(ImVec2(0,0));
	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	sf::Clock deltaClock;
	
	// Setup
	LoadTextures();
	InitLevel();
	selected_element.TEXTURE = &tex_blocks[1];
	hidden_sprite.setTexture(tex_blocks[3]);
	
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);
			if (event.type == sf::Event::Closed)
			{
                window.close();
			}
			else if(event.type == sf::Event::LostFocus)
			{
				window.setFramerateLimit(5);
			}
			else if(event.type == sf::Event::GainedFocus)
			{
				window.setFramerateLimit(60);
			}
		}
		
		ImGui::SFML::Update(window, deltaClock.restart());
		
		// Toolbox
		int app_window_height = io.DisplaySize.y;
		int app_window_width = io.DisplaySize.x;
		
		ImGui::SetNextWindowSize(ImVec2(UI.TOOLBOX_WIDTH, app_window_height - (2 * UI.MARGIN)));
		ImGui::SetNextWindowPos(ImVec2(UI.TOOLBOX_X + UI.MARGIN, UI.TOOLBOX_Y + UI.MARGIN));
		ImGui::Begin("toolbox", false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		
		// File operations
		if (ImGui::SmallButton("Open"))
		{
			auto f = pfd::open_file("Open file", DEFAULT_PATH,
									{"Level File (.osk)", "*.osk"}, false);
			
			if(!f.result().empty())
			{
				LoadLevel(f.result()[0]);
			}
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Save"))
		{
			auto f = pfd::save_file("Save file", DEFAULT_PATH,
									{"Level File (.osk)", "*.osk"}, true);
			
			if(f.result() != "")
			{
				SaveLevel(f.result());
			}
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Help"))
		{
			ImGui::OpenPopup("help");
		}
        
		bool p_help_opened = true;
		if (ImGui::BeginPopupModal("help", &p_help_opened, ImGuiWindowFlags_NoResize))
		{
			ImGui::Text("Open Solomon's Key Editor. By Immortalx. (v 1.0 05/2020)\n\n");
			ImGui::Text("How to use:");
			ImGui::Text("Select an element from the toolbox and paint it on the grid.");
			ImGui::Text("The empty block erases any previously painted elements.\n\n");
			ImGui::Text("Unique elements are those that can only be placed once in the level.");
			ImGui::Text("You have to place one of each to generate a valid level.\n\n");
			ImGui::Text("Right-click an existing element on the grid to access its properties.");
			ImGui::Text("Enemies, for example, have an initial direction and speed.\n\n");
            ImGui::Text("The Kammera mirror can be placed to prevent Dana from casting blocks,");
            ImGui::Text("but can also be set to generate either a single or two kinds of enemies.");
            
            ImGui::Text("By accessing its properties you can set the initial delay and spawn interval,");
            ImGui::Text("check 'spawn enemies' to select the number of enemies to be spawned");
            ImGui::Text("(they appear alternately in-game), and set their speed and direction.\n\n");
			ImGui::Text("There are 2 types of items in Solomon's key: Visible and hidden.");
			ImGui::Text("Those that are visible can be placed directly on the grid.");
			ImGui::Text("Hidden items can only be placed under the brown-ish destructible blocks,");
			ImGui::Text("or on any empty cell on the grid.");
			ImGui::Text("To place such an item, right-click an existing destructible block,");
			ImGui::Text("or an empty cell to access its properties.");
			ImGui::Text("Then check 'Contains hidden item' and select an item from the list.");
			ImGui::Text("An exclamation mark indicates there's a hidden item in there.");
            ImGui::Text("Hover over the mark to quickly peek the type of hidden item.\n\n");
            ImGui::Text("SOLOMON'S KEY IS A COPYRIGHT OF TECMO.\n\n");
			ImGui::Text("NOTE: This is a fun-made editor, meant to be used alongside an");
			ImGui::Text("also fun-made re-creation of the game. It's a son-father project.");
            ImGui::Text("My son did the hard work of writing the game, and I did the editor :)");
            ImGui::Text("Have fun!\n\n");
            ImGui::Text("Thanks to:");
            ImGui::Text("The SFML team");
            ImGui::Text("Omar Cornut for the Dear ImGui library");
            ImGui::Text("Sam Hocevar for Portable File Dialogs");
            ImGui::Text("StrategyWiki for their Solomon's key guide");
			if (ImGui::Button("OK", ImVec2(80,0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		
		ImGui::Text("Show/Hide grid");
		
		ImGui::Checkbox("Grid", &show_grid);
		
		// Draw selected element
		ImVec2 cur_pos = ImGui::GetCursorPos();
		ImGui::SetCursorPos(ImVec2(UI.TOOLBOX_X + 180, UI.TOOLBOX_Y + 37));
		ImGui::Text("Selected");
		ImGui::SetCursorPosX(UI.TOOLBOX_X + 174);
		ImGui::Image(*selected_element.TEXTURE);
		
		ImGui::SetCursorPos(cur_pos);
		ImGui::SetCursorPosY(cur_pos.y + 40.0f);
		ImGui::Spacing();
		
		// Blocks
		ImGui::Text("Blocks");
		
		if (ImGui::ImageButton(tex_blocks[0], ImVec2(64,64)))
		{
			selected_element.TEXTURE = &tex_blocks[0];
			selected_element.TYPE = BLOCK;
			selected_element.INDEX = 0;
		}
		ImGui::SameLine();
		if (ImGui::ImageButton(tex_blocks[1], ImVec2(64,64)))
		{
			selected_element.TEXTURE = &tex_blocks[1];
			selected_element.TYPE = BLOCK;
			selected_element.INDEX = 1;
		}
		ImGui::SameLine();
		if (ImGui::ImageButton(tex_blocks[2], ImVec2(64,64)))
		{
			selected_element.TEXTURE = &tex_blocks[2];
			selected_element.TYPE = BLOCK;
			selected_element.INDEX = 2;
		}
		
		ImGui::Spacing();
		
		
		// Unique elements
		ImGui::Text("Unique elements");
		
		if (ImGui::ImageButton(tex_unique[0], ImVec2(64,64)))
		{
			selected_element.TEXTURE = &tex_unique[0];
			selected_element.TYPE = UNIQUE;
			selected_element.INDEX = 0;
		}
		ImGui::SameLine();
		if (ImGui::ImageButton(tex_unique[1], ImVec2(64,64)))
		{
			selected_element.TEXTURE = &tex_unique[1];
			selected_element.TYPE = UNIQUE;
			selected_element.INDEX = 1;
		}
		ImGui::SameLine();
		if (ImGui::ImageButton(tex_unique[2], ImVec2(64,64)))
		{
			selected_element.TEXTURE = &tex_unique[2];
			selected_element.TYPE = UNIQUE;
			selected_element.INDEX = 2;
		}
		
		ImGui::Spacing();
		
		// Enemies list
		ImGui::Text("Enemies");
		
		if (ImGui::ListBoxHeader("##enemieslist", ImVec2(UI.LIST_WIDTH, UI.LIST_HEIGHT)))
		{   
			for (int i = 0; i < NUM_ENEMIES; ++i)
			{
				if(ImGui::Selectable(enemies_list[i]/*, active_enemy == i*/))
				{
					SetSelectedElementTexture(ENEMY, i);
					selected_element.TYPE = ENEMY;
					selected_element.INDEX = i;
				} 
				
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Image(tex_enemies[i], ImVec2(64,64));
					ImGui::EndTooltip();
				}
			}
		}
		ImGui::ListBoxFooter();
		
		// Items list
		ImGui::Text("Items");
        
		if (ImGui::ListBoxHeader("##itemslist", ImVec2(UI.LIST_WIDTH, UI.LIST_HEIGHT)))
		{   
			for (int i = 0; i < NUM_ITEMS; ++i)
			{
				if(ImGui::Selectable(items_list[i]/*, active_item == i*/))
				{
					SetSelectedElementTexture(ITEM, i);
					selected_element.TYPE = ITEM;
					selected_element.INDEX = i;
				}
				
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Image(tex_items[i], ImVec2(64,64));
					ImGui::EndTooltip();
				}
			}
		}
		ImGui::ListBoxFooter();
		
		// Backgrounds list
		ImGui::Text("Backgrounds");
		
		if (ImGui::ListBoxHeader("##backgroundslist", ImVec2(UI.LIST_WIDTH, UI.LIST_HEIGHT)))
		{   
			for (int i = 0; i < NUM_BACKGROUNDS; ++i)
			{
				if(ImGui::Selectable(std::to_string(i).c_str()))
				{
					background_index = i;
				}
				
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Image(tex_backgrounds[i], ImVec2(64,64));
					ImGui::EndTooltip();
				}
			}
		}
		ImGui::ListBoxFooter();
		
		ImGui::Spacing();
		ImGui::End();
		
		
		// Main window
		ImGui::SetNextWindowSize(ImVec2(app_window_width - UI.MAIN_X - UI.MARGIN, app_window_height - (2 * UI.MARGIN)));
		ImGui::SetNextWindowPos(ImVec2(UI.MAIN_X, UI.MAIN_Y));
		ImGui::SetNextWindowContentSize(ImVec2(960 + UI.MARGIN + 4, 768 + UI.MARGIN));
		
		// this allows the grid to be visible over the background
		float bg_alpha = 0.0f;
		ImGui::SetNextWindowBgAlpha(bg_alpha);
		
		ImGui::Begin("main", false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		
		draw_list = ImGui::GetWindowDrawList();
		
		if (show_grid)
		{
			DrawGrid();
		}
		
		// Store elements
		if (ImGui::IsWindowHovered())
		{
			int idx = GetCellIndex();
			
			// Show hidden item on hover
			if(level[idx].CONTAINS_HIDDEN)
			{
				ImGui::BeginTooltip();
				ImGui::Image(tex_items[level[idx].HIDDEN_ITEM_INDEX], ImVec2(64,64));
				ImGui::EndTooltip();
			}
			
			// Show kameera mirror enemies (if set) on hover
			if(level[idx].IS_SPAWN_GATE)
			{
				ImGui::BeginTooltip();
				ImGui::Image(tex_enemies[level[idx].ENEMY1_INDEX], ImVec2(64,64));
				if(level[idx].NUM_ENEMIES_SPAWNED == 2)
				{
					ImGui::Image(tex_enemies[level[idx].ENEMY2_INDEX], ImVec2(64,64));
				}
				ImGui::EndTooltip();
			}
			
			//Set property popup to open
			if(ImGui::IsMouseClicked(1))
			{
				property_mode = true;
				element_index = idx;
				ImGui::OpenPopup("Element Options");
			}
			
			if (ImGui::IsMouseDown(0))
			{
				if(!property_mode)
				{
					CheckForUniquePresence(idx);
					PlaceSelectedElement(idx);
				}
			}
		}
		
		// Properties popup
		OpenPropertiesWindow();
		window.clear();
        
		background_sprite.setTexture(tex_backgrounds[background_index]);
		background_sprite.setPosition(UI.GRID_START_X, UI.GRID_START_Y);
		window.draw(background_sprite);
		DrawLevel(window);
        
		ImGui::End();
		ImGui::SFML::Render(window);
		window.display();
	}
	
	ImGui::SFML::Shutdown();
}