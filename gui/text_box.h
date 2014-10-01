#ifndef TEXT_BOX
#define TEXT_BOX

#include <string>
#include <unordered_map>
#include <utility>
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

	uint8_t text_spd;
	uint8_t loop_ani;
	uint32_t loop_pos;

	SDL_Texture **psf_font;
	uint8_t psf_width;
	uint8_t psf_height;
	static unordered_map<string,pair<uint8_t,uint8_t>> psf_sizes;

	string box_text;

	static SDL_Renderer *renderer;
    static unordered_map<string,TTF_Font *> ttf_dict;
    static unordered_map<string,SDL_Texture **> psf_dict;

	string text_wrap(string str);
	void create_TTF_surf(string str);
	void create_bitmap_surf(string str);

	public:
	Text_box(uint32_t x, uint32_t y, uint32_t w, uint32_t h, string font_path, uint8_t font_size = 10);
    ~Text_box();
	bool load_font(string font_path);
	void set_renderer(SDL_Renderer *rend);
	void set_text_speed(uint8_t text_speed, bool loop = false);
	void render_text(float rot_angle = 0);
	void create_text_shadow(bool outline);
	void new_text(string str);
	void set_pos(uint32_t x, uint32_t y);
};

#endif
