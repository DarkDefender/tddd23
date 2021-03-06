#ifndef LEVEL_H
#define LEVEL_H

#include "sdl_h_func.h"
#include <pugixml.hpp>
#include <SDL2/SDL.h>

#include <iostream>
#include <vector>
#include <unordered_map>

#include "obj.h"
#include <list>
#include <btBulletDynamicsCommon.h>

#include "tile.h"

using namespace std;

class LevelZone {
	vector<vector<vector<uint32_t>>> level_tiles;  //Layer, y, x
	vector<Tile> tile_tex;
	vector<vector<SDL_Point>> zone_coll;
	list<GameObject*> obj_list;

	string zone_name;
	unsigned int zone_tile_w, zone_tile_h, zone_w, zone_h;
	static unordered_map<string,pugi::xml_document> zones;
	static unordered_map<string,SDL_Texture *> images;
	vector<bool> layer_ani;
	vector<SDL_Texture *> level_zone_layers;

	void parse_collison_obj(pugi::xml_node node, int tile_id);
	void parse_ani_frames(pugi::xml_node node, int tile_id, int first_id);
	public:
	LevelZone(string level_zone_file, SDL_Renderer *renderer);
	SDL_Texture *get_layer(unsigned int i, SDL_Renderer *renderer);
	vector<SDL_Texture *> get_layers(SDL_Renderer *renderer);
	vector<vector<SDL_Point>> get_coll_vec();
	SDL_Texture *get_coll_layer(SDL_Renderer *renderer);
	void update_textures(SDL_Renderer *renderer);
	void render_layers(SDL_Renderer *renderer, int off_x, int off_y);
	void render_layer(unsigned int index, SDL_Renderer *renderer, int off_x, int off_y);
	void del_images();
	void del_layers();
	list<GameObject*> *get_objs();

	SDL_Point get_zone_sizes();
	~LevelZone();
};

class Level {
	vector<vector<LevelZone*>> l_zone_tiles;
	SDL_Point render_offset, cur_tile;
	float render_rot, world_scale;
	list<GameObject*> obj_list;
	GameObject* focus_obj;
	vector<SDL_Point> prev_cam_vec;
	unsigned int cam_vec_id, tile_w, tile_h;
	SDL_Texture *level_texture;
	SDL_Rect tile_dim;

	bool rotate_world;

    string win_prop;

	//Bullet
	btBroadphaseInterface* broadphase;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;
	btVector3 grav_vec;

	btTriangleMesh* level_trimesh;
	btCollisionShape *mTriMeshShape;
	btRigidBody* levelRigidBody;

	void setup_bullet_world();
	void del_bullet_world();
	void get_lvl_objs();
	void create_terrain();

	void update_offset();
	void update_tile_index();
	public:
	Level(string level_file, SDL_Renderer *renderer);
	~Level();
	string get_win_prop();
	GameObject* get_player();
	btVector3 screen_to_game_coords(float x, float y);
	void update(float delta_s);
	void toggle_rotate_world();
	void draw_level(SDL_Renderer *renderer);
};


#endif
