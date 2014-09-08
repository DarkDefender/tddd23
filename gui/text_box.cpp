#include "text_box.h"

#include <string>
#include <cstring>
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

void Text_box::create_bitmap_surf(string str, SDL_Renderer *renderer){
        if (text != NULL){
		// Free the previous texture
		SDL_DestroyTexture(text);
	}

	text = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, text_rect.w, text_rect.h);

	SDL_SetTextureBlendMode(text, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(renderer, text);
    	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    	SDL_RenderClear(renderer);

        for (int i = 0; str[i] != '\0'; i++){
                SDL_Rect dest;
		dest.x = PSF_GetGlyphWidth()*i;
		dest.y = 0;
		dest.w=PSF_GetGlyphWidth();
		dest.h=PSF_GetGlyphHeight();
		SDL_RenderCopy(renderer, psf_font[str[i]], NULL, &dest);
	}

	// Change the target back to the default and then render the aux
	SDL_SetRenderTarget(renderer, NULL); //NULL SETS TO DEFAULT
}

void Text_box::create_TTF_surf(string str, SDL_Renderer *renderer){
	SDL_Color black = { 0x00, 0x00, 0x00, 0 };
	SDL_Surface *tmp_surf = TTF_RenderUTF8_Solid(ttf_font, str.c_str(), black);
	text = SDL_CreateTextureFromSurface(renderer, tmp_surf);
	//TODO fix text box sizes
	text_rect.w = tmp_surf->w;
	text_rect.h = tmp_surf->h;
	SDL_FreeSurface(tmp_surf);
}

void Text_box::render_text(string str, SDL_Renderer *renderer){
	if (!str.empty()){	
		if (!bitmap_font){
			create_TTF_surf(str, renderer);
		} else {
			create_bitmap_surf(str, renderer); 
		}
		SDL_RenderCopy(renderer, text, NULL, &text_rect);
	}
}
void Text_box::render_text(string str, SDL_Renderer *renderer, uint8_t text_speed){
	if (!str.empty()){	
		SDL_RenderCopy(renderer, text, NULL, &text_rect);
	}
}
