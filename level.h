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
	static unordered_map<string,pugi::xml_document> zones;
	static unordered_map<string,SDL_Texture *> images;
	vector<SDL_Texture *> level_zone_layers;

	void parse_collison_obj(pugi::xml_node node, int tile_id);
	public:
	LevelZone(string level_zone_file, SDL_Renderer *renderer);
	vector<SDL_Texture *> get_layers(SDL_Renderer *renderer);
	vector<vector<SDL_Point>> get_coll_vec();
	SDL_Texture *get_coll_layer(SDL_Renderer *renderer);
	void render_layers(SDL_Renderer *renderer, int off_x, int off_y);
	void del_images();
	void del_layers();
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
	unsigned int cam_vec_id;
	SDL_Texture *level_texture;
    
    bool rotate_world;

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
    void create_terrain();

	void update_offset();
	public:
	Level(string level_file, SDL_Renderer *renderer);
	~Level();
	GameObject* get_player();
	void update(float delta_s);
	void toggle_rotate_world();
	void draw_level(SDL_Renderer *renderer);
};


#endif
