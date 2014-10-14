#include <iostream>
#include <cstring> 
#include <list>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include "gui/text_box.h"
#include "timer.h"

#include "level.h"

#include "obj.h"

#include <btBulletDynamicsCommon.h>

const int WIDTH = 640;
const int HEIGHT = 480;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
int off_x = 0;
int off_y = 0;
float angle = 0;

const float SCALE_FACTOR = 80.0f;

SDL_Texture *texture = NULL;

using namespace std;

void cleanup(int exitcode){
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	exit(exitcode);
}

//TODO redo this in a better way
void change_lvl_ani(SDL_Renderer *renderer, list<Text_box*> text_object_list, Level *level, bool fade_in){
	Timer fps_cap_timer, timer;
	bool done = false;
    timer.start();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	while( !done ){
        //(Re)start the fps cap timer
		fps_cap_timer.start();
		level->draw_level(renderer);

		for (list<Text_box*>::iterator it = text_object_list.begin(); it != text_object_list.end(); it++){
			(*it)->render_text();
		}
		
		if(fade_in){
			SDL_SetRenderDrawColor(renderer,0,0,0, 255 * timer.delta_s());
		} else {
			SDL_SetRenderDrawColor(renderer,0,0,0, 255 - 255 * timer.delta_s());
		}
		SDL_RenderFillRect(renderer, NULL);

		SDL_RenderPresent(renderer);

		//Cap framerate
		int frame_ticks = fps_cap_timer.getTicks();
		if( frame_ticks < SCREEN_TICKS_PER_FRAME - frame_ticks){
			SDL_Delay( SCREEN_TICKS_PER_FRAME - frame_ticks );
		}
		if(timer.delta_s() > 1){
			done = true;
		}
	}
}

void update_screen(SDL_Renderer *renderer, list<Text_box*> text_object_list, Level *level){

	level->draw_level(renderer);

	for (list<Text_box*>::iterator it = text_object_list.begin(); it != text_object_list.end(); it++){
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
    //Initialize PNG loading 
	int imgFlags = IMG_INIT_PNG; 
	if( !( IMG_Init( imgFlags ) & imgFlags ) ) {
		cerr << "SDL_image could not initialize! SDL_image Error:" << IMG_GetError() << endl;
        TTF_Quit();
		SDL_Quit();
		return 2; 
	}
	// Create a window
	if (SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer) < 0) {
		cerr << "SDL_CreateWindowAndRenderer() failed: " << SDL_GetError() << endl;
		cleanup(2);
	}

	//Fullscreen
	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

    int w,h;
	SDL_GetWindowSize(window, &w, &h);
	int s_w = w / WIDTH;
	int s_h = h / HEIGHT;
	//TODO use scaling
    SDL_RenderSetScale(renderer,s_w,s_w);
	
	// Create test text objects
	// TODO remeber to free this up later
	Text_box *b1 = new Text_box(10,10,110,50,"../res/fonts/PROBE_10PX_TTF.ttf");
	// Only need to set renderer one time (shared between objects)
	// TODO psf generation will fail if renderer is not set!
	b1->set_renderer(renderer);
	Text_box *b2 = new Text_box(10,40,100,50,"../res/fonts/Tewi-normal-11.psf");

	b1->new_text("Hipp. Happ, hopp!");
    b2->new_text("Hipp. Happ, popp!");

	list<Text_box*> text_list;
	text_list.push_back(b1);
	text_list.push_back(b2);

    list<string> level_list;
	level_list.push_back("tutor.tmx");
	level_list.push_back("level.tmx");

    Level *level = new Level(level_list.front(), renderer);

	//btRigidBody *fallRigidBody = level->get_player()->get_body();

    GameObject *player = level->get_player();

    // Nudge the circle
	btVector3 left = btVector3(-20, 0, 0);
	btVector3 right = btVector3(20, 0, 0);
	//fallRigidBody->activate(true);
	//fallRigidBody->applyCentralImpulse(force);

	//Setup timer
	Timer fps_cap_timer;
	bool done = false;

	while ( ! done ) {
        //(Re)start the fps cap timer
		fps_cap_timer.start();

		while( SDL_PollEvent( &event ) != 0 ){  
			switch (event.type) {
				case SDL_MOUSEBUTTONDOWN:
					switch (event.button.button)
					{
						case SDL_BUTTON_LEFT:
							{
								btVector3 vec = level->screen_to_game_coords(event.button.x/s_w, event.button.y/s_w);
								player->attack(vec , 10);
								break;
							}
						case SDL_BUTTON_RIGHT:
							//SDL_ShowSimpleMessageBox(0, "Mouse", "Right button was pressed!", window);
							break;
						default:
							//TODO perhaps use this for error messages?
							//SDL_ShowSimpleMessageBox(0, "Mouse", "Some other button was pressed!", window);
							break;
					}
					/*
					off_x = - event.button.x;
					off_y = - event.button.y;
					*/
					break;
				/*
				case SDL_MOUSEMOTION:
					int mouseX = event.motion.x;
					int mouseY = event.motion.y;
					break;
					*/
				case SDL_KEYDOWN:
					if(event.key.repeat){
						// We don't want to handle key repeats
						break;
					}
					switch (event.key.keysym.sym)
					{
						case SDLK_LEFT:  player->set_move_dir(left); break;
						case SDLK_RIGHT: player->set_move_dir(right); break;
						case SDLK_UP:    player->jump(); break;
						//case SDLK_DOWN:  f += 0.5f; fallRigidBody->setFriction(f); break;
						//case SDLK_SPACE: f -= 0.5f; fallRigidBody->setFriction(f); break;
						case SDLK_SPACE: level->toggle_rotate_world(); break;
						case SDLK_RETURN:
										 if(level->get_win_prop() == "button"){
											change_lvl_ani(renderer, text_list, level, true);
											level_list.pop_front();
											delete level;
											level = new Level(level_list.front(), renderer);
											player = level->get_player();
											change_lvl_ani(renderer, text_list, level, false);
										 }
					}
					break;
 				case SDL_KEYUP:
					switch (event.key.keysym.sym)
					{
						case SDLK_LEFT:  player->set_move_dir(-left); break;
						case SDLK_RIGHT: player->set_move_dir(-right); break;
						case SDLK_UP:    player->stop_jump(); break;
						//case SDLK_SPACE: rota = !rota; break;
					}
					break;
				case SDL_QUIT:
					done = 1;
					break;
				default:
					break;
			}
		}
		update_screen(renderer, text_list, level);

		//Cap framerate
		int frame_ticks = fps_cap_timer.getTicks();
		if( frame_ticks < SCREEN_TICKS_PER_FRAME - frame_ticks){
			SDL_Delay( SCREEN_TICKS_PER_FRAME - frame_ticks );
		}

        //update physics
		level->update(fps_cap_timer.delta_s());

	}

	delete level;

	// TODO clean up all loaded fonts!
	//TTF_CloseFont(font);  
	SDL_Quit();

	return 0;
}
