#ifndef LEVEL_H
#define LEVEL_H

#include "sdl_h_func.h"
#include <pugixml.hpp>
#include <SDL2/SDL.h>

#include <iostream>
#include <vector>
#include <map>

using namespace std;

struct Tile {
	SDL_Texture * texture;
	SDL_Rect rect;
	//collition shapes for this tile. Everything is stored as polylines
	vector<vector<SDL_Point>> coll;
};

class LevelZone {
	vector<vector<vector<uint32_t>>> level_tiles;  //Layer, y, x
	vector<Tile> tile_tex;
	vector<vector<SDL_Point>> zone_coll;

	string zone_name;
	unsigned int zone_tile_w, zone_tile_h, zone_w, zone_h;
	static map<string,pugi::xml_document> zones;
	static map<string,SDL_Texture *> images;
	vector<SDL_Texture *> level_zone_layers;

	void parse_collison_obj(pugi::xml_node node, int tile_id);
	public:
	LevelZone(string level_zone_file, SDL_Renderer *renderer);
	vector<SDL_Texture *> get_layers(SDL_Renderer *renderer);
	SDL_Texture *get_coll_layer(SDL_Renderer *renderer);
	void render_layers(SDL_Renderer *renderer);
	void del_images();
	void del_layers();
	~LevelZone();
};

class Level {
	vector<vector<LevelZone>> l_zone_tiles;
	public:
	Level(string level_file);
	void draw_level();
};


#endif
