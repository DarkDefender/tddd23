#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "libs/libPSF/PSF.h"

#define WIDTH   640
#define HEIGHT  480

void cleanup(int exitcode){
	SDL_Quit();
	exit(exitcode);
}

int main(int argc, char *argv[]){
	
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Event event;

	std::cout << "Hello World!\n";

	/* Create a window */
	if (SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer) < 0) {
		std::cerr << "SDL_CreateWindowAndRenderer() failed: " << SDL_GetError() << std::endl;
		cleanup(2);
	}

	bool done = false;
	while ( ! done ) {
		if ( SDL_WaitEvent(&event) < 0 ) {
			std::cerr << "SDL_PullEvent() error: " << SDL_GetError() << std::endl;
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

	SDL_Quit();

	return 0;
}
