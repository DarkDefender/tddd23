#include "text_box.h"

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

Text_box::Text_box(uint32 x, uint32, y, uint32 w, uint32 h, const TTF_font *ttf_font){
	text_rect.x = x;
	text_rect.y = y;
	text_rect.w = w;
	text_rect.h = w;
	text = str;
	animation_done = true;
	font = ttf_font;
	bitmap_font = false;
}

Text_box::Text_box(uint32 x, uint32, y, uint32 w, uint32 h, const SDL_Texture *psf_font){
	text_rect.x = x;
	text_rect.y = y;
	text_rect.w = w;
	text_rect.h = w;
	text = str;
	animation_done = true;
	font = psf_font;
	bitmap_font = true;
}

void Text_box::render_text(string str, SDL_Renderer *renderer){
	SDL_RenderCopy(renderer, text, NULL, &text_rect);
}
void Text_box::render_text(string str, SDL_Renderer *renderer, uint8 text_speed){
	SDL_RenderCopy(renderer, text, NULL, &text_rect);
}
