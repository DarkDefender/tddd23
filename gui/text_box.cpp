#include "text_box.h"

#include <string>
#include <algorithm>
#include <map>
#include <cstring>
#include <iostream>
#include <sstream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "libs/libPSF/PSF.h"

using namespace std;

Text_box::Text_box(uint32_t x, uint32_t y, uint32_t w, uint32_t h, string font_path, uint8_t font_size){
	text_rect.x = x;
	text_rect.y = y;
	text_rect.w = w;
	text_rect.h = h;
	animation_done = true;
	texture = NULL;
    // size is only used for the ttf font
	ttf_size = font_size;
	ttf_font = NULL;
	psf_font = NULL;
	box_text = "You forgot to add a text!";
	load_font(font_path);
}

// TODO perhaps remove the set renderer function and just init this one instead
SDL_Renderer *Text_box::renderer = NULL;
map<string,TTF_Font *> Text_box::ttf_dict;
map<string,SDL_Texture **> Text_box::psf_dict;
		
void Text_box::set_renderer(SDL_Renderer *rend){
	renderer = rend;
}

bool Text_box::load_font(string font_path){
	string font_type = font_path.substr(font_path.find_last_of(".") + 1);
	transform(font_type.begin(), font_type.end(), font_type.begin(), ::tolower);
	if (font_type == "psf") {

		bitmap_font = true;

        if (psf_dict.count(font_path) > 0){
			//Only load a font once
			psf_font = psf_dict[font_path];
			return true;
		}

		// Init PSF font
		PSF_OpenFont(font_path.c_str());

		PSF_ReadHeader();

        // Create new texture array to strore the glyphs
		psf_font = new SDL_Texture*[512];

		//PSF_GetGlyphTotal tells us how many glyphs are in the font
		for (int i=0; i < PSF_GetGlyphTotal(); i++)
		{
			//Create a surface of exactly the right size for each glyph
			SDL_Surface *tmp=SDL_CreateRGBSurface(0,PSF_GetGlyphWidth(),PSF_GetGlyphHeight(),32,0xFF000000,0x00FF0000,0x0000FF00,0x000000FF);

			//Read the glyph directly into the surface's memory
			PSF_ReadGlyph(tmp->pixels,4,0x000000FF,0x00000000);

			//Convert the surface to a texture
			psf_font[i]=SDL_CreateTextureFromSurface(renderer,tmp);

			//Free the surface's memory
			SDL_FreeSurface(tmp);
		}

		//PSF Font loaded to textures, close the original file
		PSF_CloseFont();

		//Store the PSF font
		psf_dict[font_path] = psf_font;

	} else if (font_type == "ttf") {
		bitmap_font = false;

        if (ttf_dict.count(font_path) > 0){
			//Only load a font once
			ttf_font = ttf_dict[font_path];
			return true;
		}

		TTF_Font *font = TTF_OpenFont(font_path.c_str(), ttf_size);
		//TODO Better handling when font loading fails perhaps?
		if ( font == NULL ) {
			cerr << "Couldn't load " << ttf_size << " pt font from " << font_path << ": " << SDL_GetError() << endl;
			return false;
		}
		TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
		TTF_SetFontOutline(font, 0);
		TTF_SetFontKerning(font, 1);
		TTF_SetFontHinting(font, TTF_HINTING_NORMAL);

		//Store the loaded TTF font
		ttf_dict[font_path] = font;

		ttf_font = font;

	} else {
      cerr << "Unsupported font type: " << font_type << " Path: " << font_path << endl;
	  return false;
	}
	return true;
}

string Text_box::text_wrap(string str){
	istringstream iss(str);
	string word, wrapped_str = "";
	int lenght = 0, space_width = 0;

	if (!bitmap_font){
		string space = " ";
		TTF_SizeUTF8(ttf_font, space.c_str(), &space_width, NULL);
	} else {
        space_width = PSF_GetGlyphWidth();
	}

	while(iss >> word) {
		int word_len = 0;
		if(bitmap_font){
			word_len = word.length();
			lenght += (word_len * PSF_GetGlyphWidth());
		} else {
			TTF_SizeUTF8(ttf_font, word.c_str(), &word_len, NULL);
			lenght += word_len;
		}
		if (wrapped_str == ""){
			wrapped_str = word;
		} else if (lenght > text_rect.w){
			wrapped_str += "\n" + word;
			lenght = word_len;
		} else {
			wrapped_str += " " + word;
		}
		// Account for space that will be added
		lenght += space_width;
	}
	return wrapped_str;
}

void Text_box::create_bitmap_surf(string str){

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, text_rect.w, text_rect.h);

	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(renderer, texture);
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
		SDL_RenderCopy(renderer, psf_font[(unsigned char)str[i]], NULL, &dest);
		x++;
	}

	// Change the target back to the default and then render the aux
	SDL_SetRenderTarget(renderer, NULL); //NULL SETS TO DEFAULT
}

void Text_box::create_TTF_surf(string str){
	SDL_Color black = { 0x00, 0x00, 0x00, 0 };

    stringstream str_stream(str);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, text_rect.w, text_rect.h);

	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(renderer, texture);
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
	SDL_RenderClear(renderer);  

    int i = 0, line_h = TTF_FontLineSkip(ttf_font);

	while(getline(str_stream,str)){

		SDL_Rect dest;

		SDL_Surface *tmp_surf = TTF_RenderUTF8_Solid(ttf_font, str.c_str(), black);
		SDL_Texture *text_line = SDL_CreateTextureFromSurface(renderer, tmp_surf);

		dest.x = 0;
		dest.y = line_h * i;
		dest.w = tmp_surf->w;
		dest.h = tmp_surf->h;

		SDL_RenderCopy(renderer,text_line, NULL, &dest);

		SDL_FreeSurface(tmp_surf);
		SDL_DestroyTexture(text_line);
		i++;
	}

	// Change the target back to the default and then render the aux
	SDL_SetRenderTarget(renderer, NULL); //NULL SETS TO DEFAULT
}

void Text_box::new_text(string str){
	if (!str.empty()){

        box_text = str;

		if (texture != NULL){
			// Free the previous texture
			SDL_DestroyTexture(texture);
		}
		//Wrap the text
		str = text_wrap(str);

		if (!bitmap_font){
			create_TTF_surf(str);
		} else {
			create_bitmap_surf(str); 
		}
	}
}

void Text_box::render_text(){
	if (texture == NULL){
		cerr << "Tried to render text_box without texture!\n";
	}
	SDL_RenderCopy(renderer, texture, NULL, &text_rect);
}

void Text_box::render_text(string str, uint8_t text_speed){
	if (!str.empty()){	
		SDL_RenderCopy(renderer, texture, NULL, &text_rect);
	}
}
