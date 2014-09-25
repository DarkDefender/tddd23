#include "sdl_h_func.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>

using namespace std;

SDL_Texture* loadTexture( string path, SDL_Renderer *gRenderer ) {
	//The final texture 
	SDL_Texture* newTexture = NULL;
	//Load image at specified path 
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL ) {
		cerr << "Unable to load image " << path << " ! SDL_image Error: " << IMG_GetError() << endl; 
	} else { 
		//Create texture from surface pixels 
		newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL ) {
			cerr << "Unable to create texture from " << path <<" ! SDL Error: " << SDL_GetError() << endl; 
		} 
		//Get rid of old loaded surface 
		SDL_FreeSurface( loadedSurface ); 
	} 
	return newTexture;
}
