#include "level.h"
#include "sdl_h_func.h"
#include <pugixml.hpp>
#include <SDL2/SDL.h>

#include <iostream>
#include <cstring>
#include <vector>
#include <map>
#include <stdlib.h>

using namespace std; 

map<string,pugi::xml_document> LevelZone::zones;
map<string,SDL_Texture *> LevelZone::images;

Level::Level(string level_file){
	pugi::xml_document level;
	
	//TODO handle faliures
	pugi::xml_parse_result result = level.load_file(level_file.c_str());
	int level_w, level_h;

    pugi::xml_node map = level.child("map");

	level_w = atoi(map.attribute("width").value());
	level_h = atoi(map.attribute("height").value());
	//load in the level tile numbers
	/*
	int i = 1;
	vector<LevelZone> tile_vec;
	for( pugi::xml_node tiles = map.child("layer").child("data").first_child(); tiles; tiles = tiles.next_sibling()) {
        if( i == level_w){
            l_zone_tiles.push_back(tile_vec);
			tile_vec.clear();
            i = 1;
            continue;
        } else {
            tile_vec.push_back(tiles.attribute("gid").value());
        }
        i++;
    }
    */
	//TODO get filenames from gids to load level tiles
}

void Level::draw_level(){

}

LevelZone::LevelZone(string level_zone_file, SDL_Renderer *renderer){
	zone_name = level_zone_file;
	//Only have one copy of the same zone loaded
	if (zones.count(level_zone_file) == 0){
		//TODO handle faliures
		pugi::xml_parse_result result = zones[level_zone_file].load_file(level_zone_file.c_str());
	}
	pugi::xml_node map = zones[zone_name].child("map");

	zone_tile_w = atoi(map.attribute("tilewidth").value());
	zone_tile_h = atoi(map.attribute("tileheight").value());
	zone_w = atoi(map.attribute("width").value());
	zone_h = atoi(map.attribute("height").value());

	// Load layer data
	for (pugi::xml_node node = map.child("layer"); node; node = node.next_sibling("layer")) {
		//load in the level tile numbers
		unsigned int i = 1;
		vector<vector<unsigned int>> zone_tiles;
		vector<unsigned int> tile_vec;
		for( pugi::xml_node tiles = node.child("data").first_child(); tiles; tiles = tiles.next_sibling()) {
			if( i == zone_w){
				zone_tiles.push_back(tile_vec);
				tile_vec.clear();
				i = 1;
				continue;
			} else {
				tile_vec.push_back(atoi(tiles.attribute("gid").value()));
			}
			i++;
		}
		level_tiles.push_back(zone_tiles);
	}

    // Load all images required for this zone
	for (pugi::xml_node node = map.child("tileset"); node; node = node.next_sibling("tileset")) {
		int tile_w, tile_h;
		tile_w = atoi(node.attribute("tilewidth").value());
		tile_h = atoi(node.attribute("tileheight").value());
		for( pugi::xml_node node2 = node.first_child(); node2; node2 = node2.next_sibling()) {
			Tile cur_tile;
			cur_tile.rect.x = 0;
			cur_tile.rect.y = 0;
			string img_path;
            string node_name( node2.name() );
			// Zero if string is equal
			if( node_name.compare("tile") == 0 ){
				img_path = node2.child("image").attribute("source").value();
				cur_tile.rect.w = atoi(node2.child("image").attribute("width").value());
				cur_tile.rect.h = atoi(node2.child("image").attribute("height").value());
			} else {
				// This is a tileset with just one image
				img_path = node2.attribute("source").value();
				cur_tile.rect.w = atoi(node2.attribute("width").value());
				cur_tile.rect.h = atoi(node2.attribute("height").value());
			}
            
			SDL_Texture * texture;

			//Have we already loaded this texture?
			if (images.count(img_path) > 0){
				texture = images[img_path];
			} else {
				texture = loadTexture(img_path, renderer);
				images[img_path] = texture;
			}

			if(cur_tile.rect.w > tile_w || cur_tile.rect.h > tile_h){
				for (int y = 0; (cur_tile.rect.h / (tile_h * (1+y))) >= 1; y++){
					for (int x = 0; (cur_tile.rect.w / (tile_w * (1+x))) >= 1; x++){
                        Tile sub_tile;
						sub_tile.texture = texture;
						sub_tile.rect.w = tile_w;
						sub_tile.rect.h = tile_h;
						sub_tile.rect.x = x*tile_w; 
						sub_tile.rect.y = y*tile_h;
						tile_tex.push_back(sub_tile);
					}
				}
			} else {
				cur_tile.texture = texture;
				tile_tex.push_back(cur_tile);
			}
		}
	}

}

vector<SDL_Texture *> LevelZone::get_layers(SDL_Renderer *renderer){
	if( level_zone_layers.empty() ){
		for (unsigned int i = 0; i < level_tiles.size(); i++){
			SDL_Texture *texture;
			texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, zone_tile_w * zone_w, zone_tile_h * zone_h);
			SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
			SDL_SetRenderTarget(renderer, texture);
			SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
			SDL_RenderClear(renderer);
            
			for (unsigned int y = 0; y < level_tiles[i].size(); y++){
				for (unsigned int x = 0; x < level_tiles[i][y].size(); x++){
                    unsigned int index = level_tiles[i][y][x];
					if (index == 0) {
						//Empty tile
						continue;
					}
					SDL_Rect dest =  tile_tex[index -1].rect;
					dest.x = x * zone_tile_w;
					//Because we want the origin of the tile to be at the bottom left of the tile, we have to add an offset here
					dest.y = (y + 1) * zone_tile_h - tile_tex[index -1].rect.h;
					SDL_RenderCopy(renderer, tile_tex[index -1].texture, &tile_tex[index -1].rect, &dest);
				}
			}
			level_zone_layers.push_back(texture);
			// Change the target back to the default and then render the aux
			SDL_SetRenderTarget(renderer, NULL); //NULL SETS TO DEFAULT
		}
	}
	return level_zone_layers;
}

void LevelZone::render_layers(SDL_Renderer *renderer){
	if( level_zone_layers.empty() ){
		get_layers(renderer);
	} 
	SDL_Rect dest;
	dest.x = 0;
	dest.y = 0;
	dest.w = zone_tile_w * zone_w;
	dest.h = zone_tile_h * zone_h;
	for (auto it = level_zone_layers.begin(); it != level_zone_layers.end(); ++it){
		SDL_RenderCopy(renderer, *it, NULL, &dest);
	}
}

//TODO only clean up images that are not needed for the next level if needed
void LevelZone::del_images(){
	 for (auto it = images.begin(); it != images.end(); ++it){
          SDL_DestroyTexture( it->second );
		  images.erase(it);
	 }
}

void LevelZone::del_layers(){
	//Clean up all layer textures
	 for (auto it = level_zone_layers.begin(); it != level_zone_layers.end(); ++it){
          SDL_DestroyTexture( *it );
	 }
     level_zone_layers.clear();
}

LevelZone::~LevelZone(){
	del_layers();
}
