#ifndef OBJ_H
#define OBJ_H

#include <SDL2/SDL.h>
#include <btBulletDynamicsCommon.h>
#include <unordered_map>
#include <vector>
#include "sdl_h_func.h"
#include <math.h> 
#include "timer.h"
#include "tile.h"

#define BIT(x) (1<<(x))
enum collisiontypes {
    COL_NOTHING = 0, //<Collide with nothing
    COL_PLAYER = BIT(0), //<Collide with player
    COL_WALL = BIT(1), //<Collide with walls
    COL_NPC = BIT(2), //<Collide with npcs
	COL_OBJ = BIT(3), // Collide with objects 
	COL_DEAD = BIT(4) // Collide with dead stuff 
};

static const int playerCollidesWith = COL_WALL | COL_OBJ;
static const int wallCollidesWith = COL_PLAYER | COL_NPC | COL_OBJ | COL_DEAD;
static const int objCollidesWith = COL_PLAYER | COL_WALL | COL_NPC | COL_OBJ;
static const int npcCollidesWith = COL_NPC | COL_WALL | COL_OBJ;
static const int deadCollidesWith = COL_WALL;

class GameObject {
	btRigidBody* phys_body;
	btCollisionShape* body_shape;

    Tile tile;
    vector<Tile> *tiles;
	vector<SDL_Point> shoot_vec;

	Timer jump_timer, move_timer, hit_timer;	

	btVector3 move_vec, old_move_vec, adj_move_vec, jump_vec;
	float cur_move_speed, spawn_rot, spawn_x, spawn_y;

	static GameObject* player_obj;

	static unordered_map<string,btCollisionShape*> obj_coll_shape;
	static btDiscreteDynamicsWorld* phys_world;
	static SDL_Renderer *renderer;
	bool godmode, inited, moving, jumping, controllable, dead, npc;
	int health;
	void clean_up();
	void pre_init(string body_type);
	public:
	GameObject();
	GameObject( string body_type, Tile tile, vector<Tile> *tiles_ptr, uint8_t health, float x, float y, float rot, bool is_controllable = false, bool npc = false, bool god_mode = false );
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

    int get_hp();
	int get_max_hp();

    void npc_think();

	void QuaternionToEulerXYZ(const btQuaternion &quat,btVector3 &euler);

	btRigidBody *get_body();
	SDL_Point get_pos(float scale);
};

#endif
