#include "game_ui_controller.h"
#include    <iostream>


sf::Sprite* load_spirite_and_texture(std::string path, float scale=0.125f)
{
    sf::Texture* texture = new sf::Texture();
    sf::Sprite* sprite = new sf::Sprite();

    if ( texture->loadFromFile("./source/" + path)  )
    {
        texture->setSmooth(true);

        sprite->setTexture(*texture);
        sprite->setScale(scale, scale); //scale down by 8 times to 64px
        sprite->setPosition(0, 0);
    }
    else
    {
        std::cerr<<"Error while loading image as texture"<<std::endl;
    }

    return sprite;
}

void GameUiController::load_board_tiles_and_add_to_render()
{
    for(int i =0; i<10; i++)
    {
        for (int j =0 ; j<10; j++)
        {
            if (i %2 == 0 && j % 2== 0 || i %2 == 1 && j % 2== 1 )
            {
                sf::Sprite* sp = load_spirite_and_texture("tile_0.png");
                sp->setPosition(j*64, i*64);
                m_renderer_ref->gr_add_sprite_to_rendering(sp,0);
            }
            else
            {
                sf::Sprite* sp = load_spirite_and_texture("tile_1.png");
                sp->setPosition(j*64, i*64);
                m_renderer_ref->gr_add_sprite_to_rendering(sp,0);
            }
            
        }
    }
}

void GameUiController::highlight_tile(std::pair<int, int> tile)
{
    sf::Sprite* sp = load_spirite_and_texture("tile_2.png");
    sp->setPosition(tile.first*64, tile.second*64);
    m_renderer_ref->gr_add_sprite_to_rendering(sp,0);
    m_highlighted_tile = sp;
}

void GameUiController::un_highlight_tile()
{
    m_renderer_ref->gr_remove_sprite_from_rendering(m_highlighted_tile,0);
}

void GameUiController::update_mouse_coordinates()
{
    m_mouse_coords = sf::Mouse::getPosition( (*m_window) );
}

std::pair<int, int> GameUiController::coords_to_row_col(sf::Vector2f coords)
{
    int tile_width = 64;
    int tile_height = 64;
    int board_offset_horizontal = 0;
    int board_offset_vertical = 0;

    int row = ( coords.x - board_offset_horizontal ) / tile_width;
    int col = ( coords.y - board_offset_vertical ) / tile_height;

    std::cerr<<"Converting coords: "<<coords.x<<" "<<coords.y<<" to rowcol: "<<row<<" "<<col<<std::endl;
    
    if (row < 10 && col < 10)
        return std::make_pair(row, col);
    
    return std::make_pair(-1, -1);
}

void GameUiController::check_game_over()
{  
    //Check if game should be ended
    if(m_gameplay_controller_ref->gc_is_game_done() != 0)
    {
        int result = m_gameplay_controller_ref->gc_is_game_done();
        if(result == 1)
        {
            std::cerr<<"White won"<<std::endl;
        }
        else
        {
            std::cerr<<"Black won"<<std::endl;
        }
    }
}

void GameUiController::resolve_frame_events()
{
    //printf("S");
    sf::Event event;
    m_window->pollEvent(event);
    update_mouse_coordinates();

    
    
    //Update sprite coordinates if mouse is holding something
    if(m_is_piece_grabbed == true)
    {
        sf::Vector2i c = m_mouse_coords;
        c.x -= 32;
        c.y -= 32;
        m_grabbed_piece->pi_get_sprite()->setPosition(m_window->mapPixelToCoords(c));
    }

    if (event.type == sf::Event::Closed) m_window->close();
    
    if (event.type == sf::Event::MouseButtonPressed)
    {
        std::cerr<<"Mosue clicked at: "<<m_mouse_coords.x << ", "<<m_mouse_coords.y<<std::endl;
        //HANDLE BUTTONS

        //TRY TO GRAB
        if (m_is_piece_grabbed == false)
        {
            auto grabbed_piece = m_gameplay_controller_ref->gc_try_to_get_piece_at_coords(m_window->mapPixelToCoords(m_mouse_coords));
            if(grabbed_piece != nullptr)
            {
                if(grabbed_piece->pi_get_move_direction() == m_whose_turn)
                {
                    std::cerr<<"Piece just got grabbed"<<std::endl;
                    m_is_piece_grabbed = true;
                    m_grabbed_piece = grabbed_piece;
                    m_origin_tile = coords_to_row_col(m_window->mapPixelToCoords(m_mouse_coords));
                }
                else
                {
                    std::cerr<<"Tried to grab wrong player's piece"<<std::endl;
                }
            }
        }
        else
        {
            //TRY TO PLACE IT
            std::pair<int, int> rowcol = coords_to_row_col(m_window->mapPixelToCoords(m_mouse_coords));
            std::cerr<<"Trying to move from "<<m_origin_tile.first<<","<<m_origin_tile.second<<" to "
                                <<rowcol.first<<","<<rowcol.second<<std::endl;

            if(rowcol.first > 0) //check if legal move TODO 
            {
                bool successfull_move;

                if(m_has_to_attack)
                    successfull_move = m_gameplay_controller_ref->gc_make_mandatory_attack_move(m_origin_tile, rowcol, m_has_to_attack_tile);
                else
                    successfull_move = m_gameplay_controller_ref->gc_make_move(m_origin_tile, rowcol);


                if(successfull_move)
                {
                    
                    std::cerr<<"Dropped piece at "<<rowcol.first<<","<<rowcol.second<<std::endl;
                    m_grabbed_piece->pi_get_sprite()->setPosition(rowcol.first*64, rowcol.second*64);
                    bool killed_any = false;
                    if( m_gameplay_controller_ref->gc_get_dead_piece_pointer() != nullptr)
                    {
                        std::cerr<<"Piece has been killed"<<std::endl;
                        killed_any = true;
                        piece_killed_ui_broadcast(m_gameplay_controller_ref->gc_get_dead_piece_pointer()->pi_get_piece_color());
                        
                        m_renderer_ref->gr_remove_sprite_from_rendering(
                            m_gameplay_controller_ref->
                            gc_get_dead_piece_pointer()->
                            pi_get_sprite(), 
                            1
                            ); 

                        m_gameplay_controller_ref->gc_reset_dead_piece_pointer(); 
                        un_highlight_tile();
                        check_game_over();
                    }

                    if(killed_any)
                    {
                        //Check if has to attack further
                        m_has_to_attack_tile = m_gameplay_controller_ref->gc_check_if_has_to_attack(m_whose_turn);
                        m_has_to_attack = (m_has_to_attack_tile.first == -1) ? false : true;
                    }

                    if(!m_has_to_attack)
                    {
                        m_whose_turn*=-1;
                    }

                    //Check if round player has to attack in his turn
                    m_has_to_attack_tile = m_gameplay_controller_ref->gc_check_if_has_to_attack(m_whose_turn);
                    m_has_to_attack = (m_has_to_attack_tile.first == -1) ? false : true;

                    if(m_has_to_attack)
                    {
                        //Do something so he have to attack
                        //If next player is obliged to attack, allow only this move and highlight tile
                        if(m_has_to_attack)
                        {
                            std::cerr<<"Has to attack tile: "<<m_has_to_attack_tile.first<<" "<<m_has_to_attack_tile.second<<std::endl;
                            highlight_tile(m_has_to_attack_tile);
                        }
                    }
                }
                else 
                {
                    std::cerr<<"ILLEGAL MOVE from "<<m_origin_tile.first<<","<<m_origin_tile.second<<" to "
                                <<rowcol.first<<","<<rowcol.second<<std::endl;
                    m_grabbed_piece->pi_get_sprite()->setPosition(m_origin_tile.first*64, m_origin_tile.second*64);
                }
                m_grabbed_piece = nullptr;
                m_is_piece_grabbed = false;
                m_origin_tile = std::make_pair(0,0);
                
            }

        }
    }

    
}

void GameUiController::load_all_ui_background_visuals()
{
    //BACKGROUND
    //--------------------------------------
    sf::Sprite* sp = load_spirite_and_texture("background.png", 1.f);
    sp->setPosition(0, 0);
    m_renderer_ref->gr_add_sprite_to_rendering(sp, 0);
    //--------------------------------------

    //BUTTONS
    //--------------------------------------
    int buttons = 3;
    std::string button_names[buttons] = {
        "Reset",
        "Toggle sound",
        "Exit"
    };

    int start_x = 768;
    int start_y = 0;
    int break_between_buttons = 32;
    for (int i =0 ; i< buttons; i++)
    {
        sf::Sprite* sp = load_spirite_and_texture("button.png", 0.5f);
        sp->setPosition(start_x, start_y + 64*i + break_between_buttons);
        m_renderer_ref->gr_add_sprite_to_rendering(sp, 1);
        m_ui_buttons.push_back(std::make_pair(button_names[i], sp));
    }
    //--------------------------------------

    //FONT
    //--------------------------------------
    m_font.loadFromFile("./source/Raleway-Medium.ttf");
    //--------------------------------------


    int white_counter_y = 512;
    int black_counter_y = 576;
    //TEXT COUNTERS 
    //--------------------------------------
    sf::Text dead_white_pieces_cnt;
    dead_white_pieces_cnt.setPosition(start_x + 64, white_counter_y + 8);
    dead_white_pieces_cnt.setCharacterSize(32);
    dead_white_pieces_cnt.setFont(m_font);
	dead_white_pieces_cnt.setFillColor(sf::Color::Black);
    dead_white_pieces_cnt.setString("0");

    m_ui_texts.push_back( std::make_pair("dead_white", dead_white_pieces_cnt) );
    m_renderer_ref->gr_add_text_to_rendering(dead_white_pieces_cnt, 1);


    sf::Text dead_black_pieces_cnt = dead_white_pieces_cnt;
    dead_black_pieces_cnt.setPosition(start_x + 64, black_counter_y + 8);

    m_ui_texts.push_back( std::make_pair("dead_black", dead_black_pieces_cnt) );
    m_renderer_ref->gr_add_text_to_rendering(dead_black_pieces_cnt, 1);
    //--------------------------------------

    //GRAPHIC PIECE FOR COUNTERS 
    //--------------------------------------
    sf::Sprite* white_pc = load_spirite_and_texture("white_piece.png");
    white_pc->setPosition(start_x, white_counter_y);
    m_renderer_ref->gr_add_sprite_to_rendering(white_pc, 1);

   
    sf::Sprite* black_pc = load_spirite_and_texture("black_piece.png");
    black_pc->setPosition(start_x, black_counter_y);
    m_renderer_ref->gr_add_sprite_to_rendering(black_pc, 1);

 
    //--------------------------------------
    

}

void GameUiController::piece_killed_ui_broadcast(PieceColor pclr)
{
    std::string color_string;
    if(pclr == PieceColor::white)
        color_string = "white";
    else if (pclr == PieceColor::black)
        color_string = "black";

    update_kill_counter(color_string);
}
void GameUiController::update_kill_counter(std::string color_string)
{
    std::string full_text_string = "dead_" + color_string;
    for(int i =0; i < m_ui_texts.size(); i++)
    {
        if ( m_ui_texts[i].first == full_text_string)
        {
            std::string s = m_ui_texts[i].second.getString();
            int current = atoi(s.c_str());

            m_ui_texts[i].second.setString(std::to_string(++current));
            std::cerr<<"Counter "<<full_text_string<< " updated to "<<current<<std::endl;
        }
    }
}




