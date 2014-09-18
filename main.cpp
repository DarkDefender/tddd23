#include <iostream>
#include <cstring> 
#include <list>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "gui/text_box.h"
#include "timer.h"

const int WIDTH = 640;
const int HEIGHT = 480;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;

using namespace std;

void cleanup(int exitcode){
	TTF_Quit();
	SDL_Quit();
	exit(exitcode);
}

void update_screen(SDL_Renderer *renderer, list<Text_box*> game_object_list){
    /* Clear the background to background color */
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    for (list<Text_box*>::iterator it = game_object_list.begin(); it != game_object_list.end(); it++){
    	(*it)->render_text();
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]){
	
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Event event;

	cout << "Hello World!\n";

	// Init the TTF lib
	if ( TTF_Init() < 0 ) {
		cerr << "Couldn't initialize TTF: " << SDL_GetError() << endl;
		SDL_Quit();
		return 2;
	}

	// Create a window
	if (SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer) < 0) {
		cerr << "SDL_CreateWindowAndRenderer() failed: " << SDL_GetError() << endl;
		cleanup(2);
	}

	// Create test text objects
	// TODO remeber to free this up later
	Text_box *b1 = new Text_box(10,10,110,50,"../res/fonts/PROBE_10PX_TTF.ttf");
	// Only need to set renderer one time (shared between objects)
	// TODO psf generation will fail if renderer is not set!
	b1->set_renderer(renderer);
	Text_box *b2 = new Text_box(10,40,100,50,"../res/fonts/Tewi-normal-11.psf");

	b1->new_text("Hipp. Happ, hopp!");
    b2->new_text("Hipp. Happ, popp!");

	list<Text_box*> obj_list;
	obj_list.push_back(b1);
	obj_list.push_back(b2);

	update_screen(renderer, obj_list);

    Timer fps_timer;
	Timer fps_cap_timer;
	bool done = false;
    int counted_frames = 0;
	fps_timer.start();

	while ( ! done ) {

        //(Re)start the fps cap timer
		fps_cap_timer.start();

		while( SDL_PollEvent( &event ) != 0 ){  
			switch (event.type) {
				/*
				   case SDL_MOUSEBUTTONDOWN:
				   scene.messageRect.x = event.button.x - text->w/2;
				   scene.messageRect.y = event.button.y - text->h/2;
				   scene.messageRect.w = text->w;
				   scene.messageRect.h = text->h;
				   draw_scene(renderer, &scene);
				   break;
				   */
				case SDL_KEYDOWN:
				case SDL_QUIT:
					done = 1;
					break;
				default:
					break;
			}
		}
		//Calculate and correct fps
		float avg_fps = counted_frames / ( fps_timer.getTicks() / 1000.f );
		if( avg_fps > 2000000 ){
			avg_fps = 0;
		}
		b2->new_text(to_string(avg_fps));

		update_screen(renderer, obj_list);
        ++counted_frames;

		//Cap framerate
		int frame_ticks = fps_cap_timer.getTicks();
		if( frame_ticks < SCREEN_TICKS_PER_FRAME - frame_ticks){
			SDL_Delay( SCREEN_TICKS_PER_FRAME - frame_ticks );
		}
	}

	// TODO clean up all loaded fonts!
	//TTF_CloseFont(font);  
	SDL_Quit();

	return 0;
}
