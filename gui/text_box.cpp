#include "text_box.h"

#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "libs/libPSF/PSF.h"

using namespace std;

Text_box::Text_box(uint32_t x, uint32_t y, uint32_t w, uint32_t h, TTF_Font *font){
	text_rect.x = x;
	text_rect.y = y;
	text_rect.w = w;
	text_rect.h = h;
	animation_done = true;
	ttf_font = font;
	psf_font = NULL;
	bitmap_font = false;
	text = NULL;
}

Text_box::Text_box(uint32_t x, uint32_t y, uint32_t w, uint32_t h, SDL_Texture **font){
	text_rect.x = x;
	text_rect.y = y;
	text_rect.w = w;
	text_rect.h = h;
	animation_done = true;
	ttf_font = NULL;
	psf_font = font;
	bitmap_font = true;
        text = NULL;
}

// TODO perhaps remove the set function and just init this one instead
SDL_Renderer *Text_box::renderer = NULL;

void Text_box::set_renderer(SDL_Renderer *rend){
	renderer = rend;
}

string Text_box::text_wrap(string str){
	istringstream iss(str);
	string word, wrapped_str = "";
	int lenght = 0;
	while(iss >> word) {
		if(bitmap_font){
			lenght += (word.length() * PSF_GetGlyphWidth());
		} else {
			int word_len = 0;
                        TTF_SizeUTF8(ttf_font, word.c_str(), &word_len, NULL);
			lenght += word_len;
		}
		if (lenght > text_rect.w){
			wrapped_str += '\n' + word;
		} else {
			wrapped_str += " " + word;
		}
	}
	return wrapped_str;
}

void Text_box::create_bitmap_surf(string str){

	text = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, text_rect.w, text_rect.h);

	SDL_SetTextureBlendMode(text, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(renderer, text);
    	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    	SDL_RenderClear(renderer);

        for (int i = 0, x = 0, y = 0; str[i] != '\0'; i++){
		if (str[i] == '\n'){
			y++;
			x = 0;
			continue;
		}
                SDL_Rect dest;
		dest.x = PSF_GetGlyphWidth()*x;
		dest.y = PSF_GetGlyphHeight()*y;
		dest.w=PSF_GetGlyphWidth();
		dest.h=PSF_GetGlyphHeight();
		SDL_RenderCopy(renderer, psf_font[str[i]], NULL, &dest);
		x++;
	}

	// Change the target back to the default and then render the aux
	SDL_SetRenderTarget(renderer, NULL); //NULL SETS TO DEFAULT
}

void Text_box::create_TTF_surf(string str){
	SDL_Color black = { 0x00, 0x00, 0x00, 0 };
	SDL_Surface *tmp_surf = TTF_RenderUTF8_Solid(ttf_font, str.c_str(), black);
	text = SDL_CreateTextureFromSurface(renderer, tmp_surf);
	//TODO fix text box sizes
	text_rect.w = tmp_surf->w;
	text_rect.h = tmp_surf->h;
	SDL_FreeSurface(tmp_surf);
}

void Text_box::render_text(string str){
	if (!str.empty()){	
		if (text != NULL){
			// Free the previous texture
			SDL_DestroyTexture(text);
		}
		if (!bitmap_font){
			create_TTF_surf(str);
		} else {
			create_bitmap_surf(str); 
		}
		SDL_RenderCopy(renderer, text, NULL, &text_rect);
	}
}
void Text_box::render_text(string str, uint8_t text_speed){
	if (!str.empty()){	
		SDL_RenderCopy(renderer, text, NULL, &text_rect);
	}
}
