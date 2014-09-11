#ifndef TEXT_BOX
#define TEXT_BOX

#include <string>
#include <map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "libs/libPSF/PSF.h"

using namespace std;

class Text_box {
	//Text box location, width and height (in internal render screen pixels)
	SDL_Rect text_rect;
	bool animation_done, bitmap_font;
	//TODO free this texture
	SDL_Texture *texture;
	uint8_t render_layer;
	uint8_t ttf_size;
	TTF_Font *ttf_font;
	SDL_Texture **psf_font;
	string box_text;

	static SDL_Renderer *renderer;
    static map<string,TTF_Font *> ttf_dict;
    static map<string,SDL_Texture **> psf_dict;

	string text_wrap(string str);
	void create_TTF_surf(string str);
	void create_bitmap_surf(string str);

	public:
	Text_box(uint32_t x, uint32_t y, uint32_t w, uint32_t h, string font_path, uint8_t font_size = 10);
    bool load_font(string font_path);
	void set_renderer(SDL_Renderer *rend);
	void render_text();
	void new_text(string str);
	void render_text(string str, uint8_t text_speed);
};

#endif
