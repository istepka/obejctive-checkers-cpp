#include <SFML/Graphics.hpp>
#include <iostream>
#include <time.h>
#include "classes/abstract_piece.h"
#include "classes/pieces.h"
#include "classes/graphics_renderer.h"
#include "classes/gameplay_controller.h"
#include "classes/game_ui_controller.h"

// FOR NOW COMPILATION IS MANUAL
// g++ -c main.cpp 
// g++ main.o -o main -lsfml-graphics -lsfml-window -lsfml-system

//GameplayController
// implement game logic under make_move 

// GameUiController
// implement rounds 
// implement UI events
// implement buttons
// wire up with GraphicsRenderer

int main()
{
    sf::RenderWindow window(sf::VideoMode(1024, 640), "Checkers");
    GraphicsRenderer graphicsRenderer;
    GameplayController gameplayController;
    GameUiController gameUiController(&graphicsRenderer, &gameplayController, &window);

    float frames_target = 60.f;
    int frames_passed;

    while (window.isOpen())
    {
        //printf("");
        gameUiController.resolve_frame_events();
        graphicsRenderer.gr_render_whole_scene(&window); 
        //printf("");
        
        frames_passed > frames_target ? frames_passed = 0 : frames_passed++;
    }

    return 0;
}

