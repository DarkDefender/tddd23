#ifndef OBJ_H
#define OBJ_H

#include <SDL2/SDL.h>
#include <btBulletDynamicsCommon.h>
#include <unordered_map>
#include "sdl_h_func.h"

class GameObject {
	btRigidBody* phys_body;
	btCollisionShape* body_shape;

	string obj_name;
	SDL_Texture *texture;

	static unordered_map<string,btCollisionShape*> obj_coll_shape;
	static btDiscreteDynamicsWorld* phys_world;
	static SDL_Renderer *renderer;
	bool godmode, inited;
	uint32_t spawn_x, spawn_y;
	void clean_up();
	public:
	GameObject( string body_type, string tile_set, uint8_t health, uint32_t x, uint32_t y );
	~GameObject();
	void init();
	void set_renderer(SDL_Renderer *new_renderer);
	void set_phys_world(btDiscreteDynamicsWorld* world);
	void render_obj();
	btRigidBody *get_body();
};

#endif
