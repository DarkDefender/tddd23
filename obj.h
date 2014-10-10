#ifndef OBJ_H
#define OBJ_H

#include <SDL2/SDL.h>
#include <btBulletDynamicsCommon.h>
#include <unordered_map>
#include "sdl_h_func.h"
#include <math.h> 
#include "timer.h"

class GameObject {
	btRigidBody* phys_body;
	btCollisionShape* body_shape;

	string obj_name;
	SDL_Texture *texture;

	Timer jump_timer, move_timer;	

	btVector3 move_vec, old_move_vec, adj_move_vec, jump_vec;
	float cur_move_speed;

	static unordered_map<string,btCollisionShape*> obj_coll_shape;
	static btDiscreteDynamicsWorld* phys_world;
	static SDL_Renderer *renderer;
	bool godmode, inited, moving, jumping, controllable, dead;
	uint32_t spawn_x, spawn_y;
	int health;
	void clean_up();
	void pre_init(string body_type);
	public:
	GameObject( string body_type, string tile_set, uint8_t health, float x, float y, bool is_controllable = false );
	GameObject( string body_type, SDL_Texture *texture, uint8_t health, float x, float y, bool is_controllable = false );
	~GameObject();
	void init();
	void set_renderer(SDL_Renderer *new_renderer);
	void set_phys_world(btDiscreteDynamicsWorld* world);
	void render_obj(int off_x, int off_y);

	bool can_jump();
	bool can_jump(btVector3 &nor_vec);
	bool can_wall_jump(btVector3 &nor_vec);
	bool can_jump_static();

	void jump();
	void stop_jump();
	void set_move_dir(btVector3 new_vec);
	void update();

	void attack(btVector3 dir, int dmg);
	void apply_dmg(int dmg);
         
        bool get_controllable();
	bool get_dead();

	void QuaternionToEulerXYZ(const btQuaternion &quat,btVector3 &euler);

	btRigidBody *get_body();
	SDL_Point get_pos(float scale);
};

#endif
