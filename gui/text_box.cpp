#include "text_box.h"

#include <string>
#include <cstring>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

using namespace std;

Text_box::Text_box(uint32_t x, uint32_t y, uint32_t w, uint32_t h, TTF_Font *font){
	text_rect.x = x;
	text_rect.y = y;
	text_rect.w = w;
	text_rect.h = w;
	animation_done = true;
	ttf_font = font;
	psf_font = NULL;
	bitmap_font = false;
}

Text_box::Text_box(uint32_t x, uint32_t y, uint32_t w, uint32_t h, SDL_Texture *font){
	text_rect.x = x;
	text_rect.y = y;
	text_rect.w = w;
	text_rect.h = w;
	animation_done = true;
	ttf_font = NULL;
	psf_font = font;
	bitmap_font = true;
}

void Text_box::render_text(string str, SDL_Renderer *renderer){
	SDL_Color black = { 0x00, 0x00, 0x00, 0 }; 
        SDL_Surface *tmp_surf = TTF_RenderUTF8_Solid(ttf_font, str.c_str(), black);
        text = SDL_CreateTextureFromSurface(renderer, tmp_surf);
        SDL_FreeSurface(tmp_surf);
        SDL_RenderCopy(renderer, text, NULL, &text_rect);
}
void Text_box::render_text(string str, SDL_Renderer *renderer, uint8_t text_speed){
	SDL_RenderCopy(renderer, text, NULL, &text_rect);
}
