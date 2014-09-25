#ifndef SDL_H_FUNC
#define SDL_H_FUNC

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>

using namespace std;

SDL_Texture* loadTexture(string path, SDL_Renderer *renderer);

#endif
