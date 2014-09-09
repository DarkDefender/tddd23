#ifndef TEXT_BOX
#define TEXT_BOX

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "libs/libPSF/PSF.h"

using namespace std;

class Text_box {
	//Text box location, width and height (in internal render screen pixels)
	SDL_Rect text_rect;
	bool animation_done, bitmap_font;
	//TODO free this texture
	SDL_Texture *text;
	uint8_t render_layer;
	TTF_Font *ttf_font;
	SDL_Texture **psf_font;

        static SDL_Renderer *renderer;

        string text_wrap(string str);
	void create_TTF_surf(string str);
        void create_bitmap_surf(string str);

	public:
	Text_box(uint32_t x, uint32_t y, uint32_t w, uint32_t h, TTF_Font *ttf_font);
	Text_box(uint32_t x, uint32_t y, uint32_t w, uint32_t h, SDL_Texture **psf_font);
	void set_renderer(SDL_Renderer *rend);
	void render_text(string str);
	void render_text(string str, uint8_t text_speed);
};

#endif
