#include <iostream>
#include <cstring> 
#include <list>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "libs/libPSF/PSF.h"
#include "gui/text_box.h"

#define WIDTH   640
#define HEIGHT  480

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
    	(*it)->render_text("Happ",renderer);
    }

    SDL_RenderPresent(renderer);
}

TTF_Font *ttf_setup(string font_file_name, uint8_t font_size){
	TTF_Font *font = TTF_OpenFont(font_file_name.c_str(), font_size);
	//TODO Better handling when font loading fails perhaps?
	if ( font == NULL ) {
		cerr << "Couldn't load " << font_size << " pt font from " << font_file_name << ": " << SDL_GetError() << endl;
		cleanup(2);
	}
	TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
	TTF_SetFontOutline(font, 0);
	TTF_SetFontKerning(font, 1);
	TTF_SetFontHinting(font, TTF_HINTING_NORMAL);

	return font;
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

        TTF_Font *font = ttf_setup("../res/fonts/PROBE_10PX_TTF.ttf",10);

	// Create test text objects
	// TODO remeber to free this up later
	Text_box *b1 = new Text_box(10,10,100,50,font);
        list<Text_box*> obj_list;
	obj_list.push_back(b1);

        update_screen(renderer, obj_list);

	bool done = false;
	while ( ! done ) {
		if ( SDL_WaitEvent(&event) < 0 ) {
			cerr << "SDL_PullEvent() error: " << SDL_GetError() << endl;
			done = true;
			continue;
		}
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

	/*
	   SDL_FreeSurface(text);
	   TTF_CloseFont(font);  
	   */
	SDL_Quit();

	return 0;
}
