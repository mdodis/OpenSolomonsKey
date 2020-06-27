void LoadTextures()
{
	int x_offset = 0;
	int y_offset = 0;
    sf::Texture t;
    
	// Blocks
    for (int i = 0; i < NUM_BLOCKS; ++i)
	{
	    t.loadFromFile("res/blocks.png", sf::IntRect(x_offset, 0, 64, 64));
	    tex_blocks[i] = t;
	    x_offset += 64;
	}
    
    // Unique
    x_offset = 0;
    y_offset = 0;
    for (int i = 0; i < NUM_UNIQUE; ++i)
	{
	    t.loadFromFile("res/unique.png", sf::IntRect(x_offset, 0, 64, 64));
	    tex_unique[i] = t;
	    x_offset += 64;
	}
    
    // Enemies
    x_offset = 0;
    y_offset = 0;
    for (int i = 0; i < NUM_ENEMIES; ++i)
    {
        if (i > 0 && i % TEXTURE_NUM_COLUMNS == 0)
        {
            x_offset = 0;
            y_offset += 64;
        }
        
        t.loadFromFile("res/enemies.png", sf::IntRect(x_offset, y_offset, 64, 64));
	    tex_enemies[i] = t;
        x_offset += 64;
    }
    
    // Items
    x_offset = 0;
    y_offset = 0;
    for (int i = 0; i < NUM_ITEMS; ++i)
    {
        if (i > 0 && i % TEXTURE_NUM_COLUMNS == 0)
        {
            x_offset = 0;
            y_offset += 64;
        }
        
        t.loadFromFile("res/items.png", sf::IntRect(x_offset, y_offset, 64, 64));
	    tex_items[i] = t;
        x_offset += 64;
    }
    
    // Backgrounds
    for (int i = 0; i < NUM_BACKGROUNDS; ++i)
    {
        backgrounds_list[i] = i;
        
        t.loadFromFile("res/backgrounds/" + std::to_string(i) + ".png");
	    tex_backgrounds[i] = t;
    }
}

void SetSelectedElementTexture(TILE_TYPE t, int index)
{
    switch (t)
    {
        case ENEMY:
        selected_element.TEXTURE = &tex_enemies[index];
        break;
        
        case ITEM:
        selected_element.TEXTURE = &tex_items[index];
        break;
        
        default:
        selected_element.TEXTURE = &tex_blocks[1];
    }
}

void InitLevel()
{
    float xx = UI.GRID_START_X;
    float yy = UI.GRID_START_Y;
    
    int count = 0;
    
    for (int i = 0; i < 12; ++i)
    {
        for (int j = 0; j < 15; ++j)
        {
            level[count].SPRITE.setTexture(tex_blocks[0]);
            level[count].SPRITE.setPosition(xx, yy);
            
            xx += 64.0f;
            count++;
        }
        
        xx = UI.GRID_START_X;
        yy += 64.0f;
    }
}

void DrawLevel(sf::RenderWindow &w)
{
    for (int i = 0; i < 180; ++i)
    {
        w.draw(level[i].SPRITE);
        
        if(level[i].CONTAINS_HIDDEN)
        {
            sf::Vector2f pos;
            pos = level[i].SPRITE.getPosition();
            hidden_sprite.setPosition(pos);
            w.draw(hidden_sprite);
        }
    }
}

void DrawGrid()
{
    int x_offset = UI.MAIN_X + UI.MARGIN + 4 - ImGui::GetScrollX();
    int y_offset = UI.MAIN_Y + UI.MARGIN + 4 - ImGui::GetScrollY();
    int y_start = y_offset;
    
    for (int i = 0; i <= 12; ++i)
    {
        for (int j = 0; j <= 15; ++j)
        {
            draw_list->AddLine(ImVec2(x_offset, y_start), ImVec2(x_offset, y_start + UI.GRID_HEIGHT), grid_color);
            x_offset += 64;
        }
        
        x_offset = UI.MAIN_X + UI.MARGIN + 4 - ImGui::GetScrollX();
        draw_list->AddLine(ImVec2(x_offset, y_offset), ImVec2(x_offset + UI.GRID_WIDTH, y_offset), grid_color);
        y_offset += 64;
    }
}

void SaveLevel(std::string filename)
{
    // BG :  Background index
    // 0  :  Empty block, [optional] hidden item index
    // 1  :  Destructible block, [optional] hidden item index
    // 2  :  Solid block
    // 4  :  Dana, Initial direction (0 = Left, 1 = Right)
    // 6  :  Enemy, Enemy index, Speed, Direction (0 = Left, 1 = Right, 2 = Up, 3 = Down)
    // 7  :  Door
    // 8  :  Key, Key color (0 = Red, 1 = Green, 2 = Blue)
    // 9  :  Item, Item index
    
    // If Kameera Mirror wasn't set to spawn enemies, delay and interval are saved but simply ignored in-game.
    // If set to spawn enemies, first set enemy identifier (6), then Kameera mirror index (17) and finally:
    //       Delay, Interval, {Enemy1 Index, Enemy1 Speed, Enemy1 Direction},
    //       {Enemy2 Index, Enemy2 Speed, Enemy2 Direction}
    // Second enemy properties are saved only if the mirror was set to spawn 2 enemies.
    
    std::ofstream file;
    file.open(filename);
    
    if (!file)
    {
        std::cout << "Error opening file\n";
    }
    file << "#Auto-generated from osked\n";
    file << "\n";
    file << "BG ";
    file << background_index;
    file << "\n";
    
    for (int i = 0; i < 180; ++i)
    {
        if(i % 15 == 0 && i > 0)
        {
            file << "\n";
        }
        
        if(level[i].TYPE == BLOCK)
        {
            file << level[i].INDEX;
            
            if(level[i].CONTAINS_HIDDEN)
            {
                file << ",";
                file << level[i].HIDDEN_ITEM_INDEX;
            }
        }
        else if(level[i].TYPE == UNIQUE)
        {
            if(level[i].INDEX == 0) // Dana
            {
                file << 4;
                file << ",";
                file << dana_dir;
            }
            if(level[i].INDEX == 1) // Door
            {
                file << 7;
            }
            if(level[i].INDEX == 2) // Key
            {
                file << 8;
                file << ",";
                file << key_color;
            }
        }
        else if(level[i].TYPE == ENEMY)
        {
            file << 6;
            file << ",";
            file << level[i].INDEX;
            file << ",";
            
            if(level[i].INDEX == KAMEERA_MIRROR_INDEX)
            {
                file << level[i].DELAY;
                file << ",";
                file << level[i].INTERVAL;
                
                if(level[i].IS_SPAWN_GATE)
                {
                    file << ",";
                    file << "{";
                    file << level[i].ENEMY1_INDEX;
                    file << ",";
                    file << level[i].ENEMY1_SPEED;
                    file << ",";
                    file << level[i].ENEMY1_DIR;
                    file << "}";
                    
                    if(level[i].NUM_ENEMIES_SPAWNED == 2)
                    {
                        file << ",";
                        file << "{";
                        file << level[i].ENEMY2_INDEX;
                        file << ",";
                        file << level[i].ENEMY2_SPEED;
                        file << ",";
                        file << level[i].ENEMY2_DIR;
                        file << "}";
                    }
                }
            }
            else
            {
                file << level[i].SPEED;
                file << ",";
                file << level[i].DIR;
            }
        }
        else if(level[i].TYPE == ITEM)
        {
            file << 9;
            file << ",";
            file << level[i].INDEX;
        }
        
        file << " ";
        
    }
    
    file.close();
}

void LoadLevel(std::string filename)
{
    std::ifstream file;
    file.open(filename);
    std::string line;
    int value = 0;
    std::string ch;
    TILE_TYPE cell_type;
    
    char space = ' ';
    char comma = ',';
    char comment = '#';
    
    while(std::getline(file, line))
    {
        if(line.length() > 0 && line.at(0) == comment)
        {
            continue;
        }
        if(line.length() > 0 && line.at(0) == 'B')
        {
            int p = 1;
            int result = 0;
            std::string cur;
            
            for (int i = line.length() - 1; i >= 3 ; --i)
            {
                cur = line.at(i);
                result += p* std::stoi(cur);
                p *= 10;
            }
            background_index = result;
            continue;
        }
        
        int count = 0;
        while(count < line.length())
        {
            
            count++;
        }
    }
    
    file.close();
}

int GetCellIndex()
{
    int current_row = (ImGui::GetMousePos().y - UI.GRID_START_Y) / 64;
    int current_col = (ImGui::GetMousePos().x - UI.GRID_START_X) / 64;
    if (current_row > 11) current_row = 11;
    if (current_col > 14) current_col = 14;
    int idx = (current_row  * 15) + current_col;
    
    return idx;
}

void OpenPropertiesWindow()
{
    if(ImGui::BeginPopupModal("Element Options", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        int el_type = level[element_index].TYPE;
        
        // Dana initial direction
        if(el_type == UNIQUE && level[element_index].INDEX == 0)
        {
            ImGui::Text("Initial Dana direction (effective only in-game)");
            int dir = dana_dir;
            
            if(ImGui::RadioButton("Left##dd", &dir, 0))
            {
                dana_dir = dir;
            }
            if(ImGui::RadioButton("Right##dd", &dir, 1))
            {
                dana_dir = dir;
            }
        }
        // Key color
        else if(el_type == UNIQUE && level[element_index].INDEX == 2)
        {
            ImGui::Text("Key color (effective only in-game)");
            int kc = key_color;
            
            if(ImGui::RadioButton("Red", &kc, 0))
            {
                key_color = kc;
            }
            if(ImGui::RadioButton("Green", &kc, 1))
            {
                key_color = kc;
            }
            if(ImGui::RadioButton("Blue", &kc, 2))
            {
                key_color = kc;
            }
        }
        else if(el_type == ENEMY)
        {
            // Kameera mirror is a special case of enemy
            if(level[element_index].INDEX == KAMEERA_MIRROR_INDEX)
            {
                // Delay
                ImGui::Text("Initial delay");
                int delay = level[element_index].DELAY;
                if(ImGui::SliderInt("##delay", &delay, MIN_DELAY, MAX_DELAY))
                {
                    level[element_index].DELAY = delay;
                }
                
                // Interval
                ImGui::Text("Spawn interval");
                int interval = level[element_index].INTERVAL;
                if(ImGui::SliderInt("##interval", &interval, MIN_INTERVAL, MAX_INTERVAL))
                {
                    level[element_index].INTERVAL = interval;
                }
                
                bool is_spawn_gate = level[element_index].IS_SPAWN_GATE;
                if(ImGui::Checkbox("Spawn enemies", &is_spawn_gate))
                {
                    is_spawn_gate= !is_spawn_gate;
                    level[element_index].IS_SPAWN_GATE = !level[element_index].IS_SPAWN_GATE;
                }
                
                if(is_spawn_gate)
                {
                    ImGui::Text("Number of enemies");
                    int num_enemies_spawned = level[element_index].NUM_ENEMIES_SPAWNED;
                    if(ImGui::SliderInt("##Number of enemies", &num_enemies_spawned, 1, 2))
                    {
                        level[element_index].NUM_ENEMIES_SPAWNED = num_enemies_spawned;
                    }
                    
                    ImGui::Text("Enemy 1: ");
                    ImGui::SameLine();
                    ImGui::Text(enemies_list[level[element_index].ENEMY1_INDEX]);
                    
                    if (ImGui::ListBoxHeader("##enemy1", ImVec2(UI.LIST_WIDTH, UI.LIST_HEIGHT)))
                    {   
                        for (int i = 0; i < NUM_ENEMIES; ++i)
                        {
                            if(ImGui::Selectable(enemies_list[i]))
                            {
                                level[element_index].ENEMY1_INDEX = i;
                            }
                            
                            if (ImGui::IsItemHovered())
                            {
                                ImGui::BeginTooltip();
                                ImGui::Image(tex_enemies[i], ImVec2(64,64));
                                ImGui::EndTooltip();
                            }
                        }
                        ImGui::ListBoxFooter();
                    }
                    
                    ImGui::Text("Initial Speed");
                    int e1speed = level[element_index].ENEMY1_SPEED;
                    if(ImGui::SliderInt("##enemy1Speed", &e1speed, MIN_SPEED, MAX_SPEED))
                    {
                        level[element_index].ENEMY1_SPEED = e1speed;
                    }
                    
                    ImGui::Spacing();ImGui::Spacing();
                    
                    ImGui::Text("Initial direction");
                    int e1dir = level[element_index].ENEMY1_DIR;
                    
                    if(ImGui::RadioButton("Left##e1", &e1dir, 0))
                    {
                        level[element_index].ENEMY1_DIR = e1dir;
                    }
                    ImGui::SameLine();
                    if(ImGui::RadioButton("Right##e1", &e1dir, 1))
                    {
                        level[element_index].ENEMY1_DIR = e1dir;
                    }
                    if(ImGui::RadioButton("Up##e1", &e1dir, 2))
                    {
                        level[element_index].ENEMY1_DIR = e1dir;
                    }
                    ImGui::SameLine();
                    if(ImGui::RadioButton("Down##e1", &e1dir, 3))
                    {
                        level[element_index].ENEMY1_DIR = e1dir;
                    }
                    
                    if(level[element_index].NUM_ENEMIES_SPAWNED == 2)
                    {
                        ImGui::Text("Enemy 2: ");
                        ImGui::SameLine();
                        ImGui::Text(enemies_list[level[element_index].ENEMY2_INDEX]);
                        
                        if (ImGui::ListBoxHeader("##enemy2", ImVec2(UI.LIST_WIDTH, UI.LIST_HEIGHT)))
                        {   
                            for (int i = 0; i < NUM_ENEMIES; ++i)
                            {
                                if(ImGui::Selectable(enemies_list[i]))
                                {
                                    level[element_index].ENEMY2_INDEX = i;
                                }
                                
                                if (ImGui::IsItemHovered())
                                {
                                    ImGui::BeginTooltip();
                                    ImGui::Image(tex_enemies[i], ImVec2(64,64));
                                    ImGui::EndTooltip();
                                }
                            }
                            ImGui::ListBoxFooter();
                        }
                        
                        ImGui::Text("Initial Speed");
                        int e2speed = level[element_index].ENEMY2_SPEED;
                        if(ImGui::SliderInt("##enemy2Speed", &e1speed, MIN_SPEED, MAX_SPEED))
                        {
                            level[element_index].ENEMY2_SPEED = e2speed;
                        }
                        
                        ImGui::Spacing();ImGui::Spacing();
                        
                        ImGui::Text("Initial direction");
                        int e2dir = level[element_index].ENEMY2_DIR;
                        
                        if(ImGui::RadioButton("Left##e2", &e2dir, 0))
                        {
                            level[element_index].ENEMY2_DIR = e2dir;
                        }
                        ImGui::SameLine();
                        if(ImGui::RadioButton("Right##e2", &e2dir, 1))
                        {
                            level[element_index].ENEMY2_DIR = e2dir;
                        }
                        if(ImGui::RadioButton("Up##e2", &e2dir, 2))
                        {
                            level[element_index].ENEMY2_DIR = e2dir;
                        }
                        ImGui::SameLine();
                        if(ImGui::RadioButton("Down##e2", &e2dir, 3))
                        {
                            level[element_index].ENEMY2_DIR = e2dir;
                        }
                    }
                }
            }
            else
            {
                ImGui::Text("Initial Speed");
                int speed = level[element_index].SPEED;
                if(ImGui::SliderInt("##Speed", &speed, MIN_SPEED, MAX_SPEED))
                {
                    level[element_index].SPEED = speed;
                }
                
                ImGui::Spacing();ImGui::Spacing();
                
                ImGui::Text("Initial direction");
                int dir = level[element_index].DIR;
                
                if(ImGui::RadioButton("Left", &dir, 0))
                {
                    level[element_index].DIR = dir;
                }
                ImGui::SameLine();
                if(ImGui::RadioButton("Right", &dir, 1))
                {
                    level[element_index].DIR = dir;
                }
                if(ImGui::RadioButton("Up", &dir, 2))
                {
                    level[element_index].DIR = dir;
                }
                ImGui::SameLine();
                if(ImGui::RadioButton("Down", &dir, 3))
                {
                    level[element_index].DIR = dir;
                }
            }
        }
        
        else if(el_type == BLOCK && level[element_index].INDEX != 2)
        {
            bool hidden_item = level[element_index].CONTAINS_HIDDEN;
            if(ImGui::Checkbox("Contains hidden item", &hidden_item))
            {
                hidden_item = !hidden_item;
                level[element_index].CONTAINS_HIDDEN = !level[element_index].CONTAINS_HIDDEN;
            }
            
            if(hidden_item)
            {
                // Items list
                ImGui::Text("Hidden Item: ");
                ImGui::SameLine();
                ImGui::Text(items_list[level[element_index].HIDDEN_ITEM_INDEX]);
                
                if (ImGui::ListBoxHeader("##hiddenitem", ImVec2(UI.LIST_WIDTH, UI.LIST_HEIGHT)))
                {   
                    for (int i = 0; i < NUM_ITEMS; ++i)
                    {
                        if(ImGui::Selectable(items_list[i]))
                        {
                            level[element_index].HIDDEN_ITEM_INDEX = i;
                        }
                        
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::BeginTooltip();
                            ImGui::Image(tex_items[i], ImVec2(64,64));
                            ImGui::EndTooltip();
                        }
                    }
                    ImGui::ListBoxFooter();
                }
            }
        }
        
        if (ImGui::Button("Close", ImVec2(80,0)))
        {
            property_mode = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

void CheckForUniquePresence(int idx)
{
    // Prevent unique elements getting set more than once
    if(selected_element.TYPE == UNIQUE)
    {
        if(selected_element.INDEX == 0)
        {
            if(last_dana_index != -1)
            {
                level[last_dana_index].SPRITE.setTexture(tex_blocks[0]);
                level[last_dana_index].TYPE = BLOCK;
                level[last_dana_index].INDEX = 0;
            }
            last_dana_index = idx;
        }
        if(selected_element.INDEX == 1)
        {
            if(last_door_index != -1)
            {
                level[last_door_index].SPRITE.setTexture(tex_blocks[0]);
                level[last_door_index].TYPE = BLOCK;
                level[last_door_index].INDEX = 0;
            }
            last_door_index = idx;
        }
        if(selected_element.INDEX == 2)
        {
            if(last_key_index != -1)
            {
                level[last_key_index].SPRITE.setTexture(tex_blocks[0]);
                level[last_key_index].TYPE = BLOCK;
                level[last_key_index].INDEX = 0;
            }
            last_key_index = idx;
        }
    }
}

void PlaceSelectedElement(int idx)
{
    level[idx].SPRITE.setTexture(*selected_element.TEXTURE);
    level[idx].TYPE = selected_element.TYPE;
    level[idx].INDEX = selected_element.INDEX;
    
    // Reset all other properties to defaults
    level[idx].SPEED = DEFAULT_SPEED;
    level[idx].DIR = DEFAULT_DIR;
    level[idx].CONTAINS_HIDDEN = false;
    level[idx].HIDDEN_ITEM_INDEX = 0;
    
    level[idx].IS_SPAWN_GATE = false;
    level[idx].DELAY = DEFAULT_DELAY;
    level[idx].INTERVAL = DEFAULT_INTERVAL;
    level[idx].NUM_ENEMIES_SPAWNED = DEFAULT_NUM_ENEMIES_SPAWNED;
    level[idx].ENEMY1_INDEX = 0;
    level[idx].ENEMY1_SPEED = DEFAULT_SPEED;
    level[idx].ENEMY1_DIR = DEFAULT_DIR;
    level[idx].ENEMY2_INDEX = 0;
    level[idx].ENEMY2_SPEED = DEFAULT_SPEED;
    level[idx].ENEMY2_DIR = DEFAULT_DIR;
}