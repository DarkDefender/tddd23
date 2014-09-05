#ifndef TEXT_BOX
#define TEXT_BOX

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

class Text_box {
	//Text box location, width and height (in internal render screen pixels)
	SDL_Rect text_rect;
	bool animation_done, bitmap_font;
	string text;
	uint8 render_layer;
	auto *font;
	public:
	Text_box(uint32 x, uint32, y, uint32 w, uint32 h, const TTF_Font *ttf_font);
	Text_box(uint32 x, uint32, y, uint32 w, uint32 h, const SDL_Texture *psf_font);
	void render_text(string str, SDL_Renderer *renderer);
	void render_text(string str, SDL_Renderer *renderer, uint8 text_speed);
}

#endif
