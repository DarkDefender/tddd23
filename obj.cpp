#include <SDL2/SDL.h>
#include <btBulletDynamicsCommon.h>
#include <unordered_map>
#include <vector>
#include "sdl_h_func.h"
#include "obj.h"
#include <math.h>  
#include <iostream>
#include "timer.h"
#include "tile.h"

using namespace std;

unordered_map<string,btCollisionShape*> GameObject::obj_coll_shape;
btDiscreteDynamicsWorld* GameObject::phys_world = NULL;
SDL_Renderer* GameObject::renderer = NULL;
GameObject* GameObject::player_obj = NULL;

GameObject::GameObject(){
	//Only used for static init and cleanup
	inited = false;
}

GameObject::GameObject( string body_type, Tile new_tile, vector<Tile> *tiles_ptr, uint8_t start_health, float x, float y, float rot_deg, bool is_controllable, bool is_npc, bool god ){
	npc = is_npc;
    tile = new_tile;
    tiles = tiles_ptr;
	controllable = is_controllable;
	//This is spawned by a level zone so convert the coords to bullet coords (world_scale)
	spawn_x = x/80.0f;
	spawn_y = y/80.0f;
	spawn_rot = (rot_deg/180.0f)*M_PI;
	health = start_health;
	dead = false;
	godmode = god;
	pre_init(body_type);
}

void GameObject::pre_init(string body_type){
	inited = false;
	phys_body = NULL;
	moving = false;
	jumping = false;
	cur_move_speed = 0;
	jump_vec.setZero();
	move_vec.setZero();
	old_move_vec.setZero();
	adj_move_vec.setZero();
	if(obj_coll_shape.count(body_type) == 0){
		//TODO add proper shape creation based on string
		//TODO clean up shape objects when they are not needed anymore
		if(body_type == "box"){
			obj_coll_shape[body_type] = new btBoxShape(btVector3(0.5f,0.5f,0.5f));
		} else {
			obj_coll_shape[body_type] = new btCapsuleShape(0.25f, 0.5f);
			//obj_coll_shape[body_type] = new btSphereShape(0.5f);
		}
	}
	body_shape = obj_coll_shape[body_type];

	if(renderer != NULL && phys_world != NULL){
      //We have set all everything required to init!
	  init();
	}
}

void GameObject::init(){
	if(inited){
		clean_up();
	} else {
		inited = true;
	}
	btQuaternion quat;
	quat.setEuler(0, 0,spawn_rot);
	btDefaultMotionState* MotionState =
		new btDefaultMotionState(btTransform(quat , btVector3(spawn_x, spawn_y, 0)));
	btScalar mass = 10;
	btVector3 Inertia(0, 0, 0);
	//TODO calc Inertia only works on certain shapes
	body_shape->calculateLocalInertia(mass, Inertia);
	btRigidBody::btRigidBodyConstructionInfo RigidBodyCI(mass, MotionState, body_shape, Inertia);
	phys_body = new btRigidBody(RigidBodyCI);

	// Limit movement to the x,y axis. And limit rotation around the z axis
	phys_body->setLinearFactor(btVector3(1,1,0));
	if(controllable){
		phys_body->setAngularFactor(btVector3(0,0,0));
	} else {
		phys_body->setAngularFactor(btVector3(0,0,1));
	}

	//Add this GameObject to the bullet for when we do collision detection (health etc)
	phys_body->setUserPointer(this);

	//TODO only for certain situations!
	phys_body->setActivationState(DISABLE_DEACTIVATION);

	//Prevent tunneling
	//setup motion clamping so no tunneling occurs
	//phys_body->setCcdMotionThreshold(1);
	//phys_body->setCcdSweptSphereRadius(0.2f);
    
	if( controllable && !dead ){
		if( npc ){
			phys_world->addRigidBody(phys_body, COL_NPC, npcCollidesWith);
		} else {
			phys_world->addRigidBody(phys_body, COL_PLAYER, playerCollidesWith);
			player_obj = this;
		}
	} else if (dead){
			phys_world->addRigidBody(phys_body, COL_DEAD, deadCollidesWith);
	} else {
		//Can't kill objects... For now...
		godmode = true;
		phys_world->addRigidBody(phys_body, COL_OBJ, objCollidesWith);
	}
}

void GameObject::clean_up(){
	phys_world->removeRigidBody(phys_body);
	delete phys_body->getMotionState();
	delete phys_body; 
}

GameObject::~GameObject(){
	if( inited ){
		clean_up();
	}
}

void GameObject::set_renderer(SDL_Renderer *new_renderer){
	renderer = new_renderer;
}
void GameObject::set_phys_world(btDiscreteDynamicsWorld* world){
	phys_world = world;
}

btRigidBody *GameObject::get_body(){
	return phys_body;
}

SDL_Point GameObject::get_pos(float scale){
	btTransform trans;
	phys_body->getMotionState()->getWorldTransform(trans);
	SDL_Point pos = { trans.getOrigin().getX() * scale, trans.getOrigin().getY() * scale};
	return pos;
}

class ClosestNotMeSweep : public btCollisionWorld::ClosestConvexResultCallback
{
	public:
		ClosestNotMeSweep (btRigidBody* me, btVector3 from, btVector3 to) : btCollisionWorld::ClosestConvexResultCallback(from,to)
	{
		m_me = me;
	}

		virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult,bool normalInWorldSpace)
		{
			if (convexResult.m_hitCollisionObject == m_me)
				return 1.0;

			return ClosestConvexResultCallback::addSingleResult (convexResult, normalInWorldSpace);
		}
	protected:
		btRigidBody* m_me;
};

bool GameObject::can_jump(){
	btTransform pos_to, pos_from;
	btVector3 nor_grav = phys_world->getGravity().normalized();
	btSphereShape sphere(0.1f);
	phys_body->getMotionState()->getWorldTransform(pos_from);
	//Reset rotation so that we only get the origin coords
	pos_from.setRotation(btQuaternion(0,0,0));
	pos_to.setIdentity();
	pos_to.setOrigin(0.5f * nor_grav + pos_from.getOrigin());

	ClosestNotMeSweep cb( phys_body, pos_from.getOrigin(), pos_to.getOrigin() );

	phys_world->convexSweepTest(&sphere, pos_from, pos_to, cb);
	if (cb.hasHit())
	{
		return true;
	} else {
		return false;
	}
}

bool GameObject::can_jump(btVector3 &nor_vec){
	btTransform pos_to, pos_from;
	btVector3 nor_grav = phys_world->getGravity().normalized();
	btSphereShape sphere(0.1f);
	phys_body->getMotionState()->getWorldTransform(pos_from);
	//Reset rotation so that we only get the origin coords
	pos_from.setRotation(btQuaternion(0,0,0));
	pos_to.setIdentity();
	pos_to.setOrigin(0.5f * nor_grav + pos_from.getOrigin());

	ClosestNotMeSweep cb( phys_body, pos_from.getOrigin(), pos_to.getOrigin() );

	phys_world->convexSweepTest(&sphere, pos_from, pos_to, cb);
	if (cb.hasHit())
	{
		nor_vec = cb.m_hitNormalWorld;
		return true;
	} else {
		return false;
	}
}

bool GameObject::can_wall_jump(btVector3 &nor_vec){
	btSphereShape sphere(0.15f);
	btTransform pos_to, pos_from;
	phys_body->getMotionState()->getWorldTransform(pos_from);
	//Reset rotation so that we only get the origin coords
	pos_from.setRotation(btQuaternion(0,0,0));
	pos_to.setIdentity();
	
	//right wall
	pos_to.setOrigin(0.20f * phys_world->getGravity().cross(btVector3(0,0,1)).normalized() + pos_from.getOrigin());

	ClosestNotMeSweep cb( phys_body, pos_from.getOrigin(), pos_to.getOrigin() );

	phys_world->convexSweepTest(&sphere, pos_from, pos_to, cb);

	if (cb.hasHit())
	{
		nor_vec = cb.m_hitNormalWorld.rotate(btVector3(0,0,1), M_PI_4).normalized();
		return true;
	} 
	//left wall
	pos_to.setOrigin(0.20f * phys_world->getGravity().cross(btVector3(0,0,-1)).normalized() + pos_from.getOrigin());

	ClosestNotMeSweep cb2( phys_body, pos_from.getOrigin(), pos_to.getOrigin() );

	phys_world->convexSweepTest(&sphere, pos_from, pos_to, cb2);

	if (cb2.hasHit()) {
		nor_vec = cb2.m_hitNormalWorld.rotate(btVector3(0,0,1), -M_PI_4).normalized();
		return true;
	}
	return false;
}

bool GameObject::can_jump_static(){
	btSphereShape sphere(0.1f);
	btTransform pos_to, pos_from;
	phys_body->getMotionState()->getWorldTransform(pos_from);
	//Reset rotation so that we only get the origin coords
	pos_from.setRotation(btQuaternion(0,0,0));
	pos_to.setIdentity();
	pos_to.setOrigin(0.5f * phys_world->getGravity().normalized() + pos_from.getOrigin());

	btCollisionWorld::ClosestConvexResultCallback cb( pos_from.getOrigin(), pos_to.getOrigin() );
	cb.m_collisionFilterMask = btBroadphaseProxy::StaticFilter;

	phys_world->convexSweepTest(&sphere, pos_from, pos_to, cb);
	if (cb.hasHit())
	{
		return true;
	} else {
		return false;
	}
}

void GameObject::set_move_dir(btVector3 new_vec){
	old_move_vec = move_vec;
	move_vec += new_vec.normalized();
	cur_move_speed = 0;
	if(move_vec.length() < 0.1f){
		moving = false;
		move_timer.stop();
	} else {
		moving = true;
		move_timer.start();
	}
}

void GameObject::jump(){
	if(jumping){
      //We have already jumped!
	  return;
	}
	if(can_jump()){
		jumping = true;
		jump_timer.start();
		if(moving){
			jump_vec = (adj_move_vec - phys_world->getGravity()).normalized();
			phys_body->applyCentralImpulse( jump_vec * 70);
		} else {
			jump_vec = -phys_world->getGravity().normalized();
			phys_body->applyCentralImpulse( jump_vec * 70);
		}
	} else if ( can_wall_jump(jump_vec) ){
		jumping = true;
		jump_timer.start();
		phys_body->applyCentralImpulse(jump_vec * 70);
	}
}

void GameObject::stop_jump(){
	if(jumping){
		cur_move_speed = 0;
		jumping = false;
		
		float delta = jump_timer.delta_s();
		if ( delta < 0.07f ){
           delta = 0.07f;
		}

		//Only try to negate the current jumping dir if we are traveling in that dir
		if(phys_body->getLinearVelocity().angle(jump_vec) < 0.7f){ 
			phys_body->applyCentralImpulse(-jump_vec * (3.0f/delta) );
		}
		jump_timer.stop();
	}
}

void GameObject::update(){

	if(tile.animated && hit_timer.isStarted() ){
		if( !tile.ani_timer.isStarted() ){
			tile.ani_timer.start();
		} else {
			uint32_t ticks = tile.ani_timer.getTicks();
			uint32_t cur_index = tile.ani_index;
			if( ticks > tile.ani_tile_time[tile.ani_index] ){
				//(Re)start the timer
				tile.ani_timer.start();
				cur_index++;
				if(cur_index >= tile.ani_tile_no.size()){
					cur_index = 0;
				}
				tile.ani_index = cur_index;
			}
		}
		tile.texture = tiles->at(tile.ani_tile_no[tile.ani_index] - 1).texture;
		tile.rect = tiles->at(tile.ani_tile_no[tile.ani_index] - 1).rect;
	}

	if(!controllable || dead){
   		return;
	}

    if( hit_timer.delta_s() > 1 ){
		godmode = false;
		hit_timer.stop();
 		tile.texture = tiles->at(tile.ani_tile_no[0] - 1).texture;
		tile.rect = tiles->at(tile.ani_tile_no[0] - 1).rect;  
	}

	if( npc ){
		npc_think();
	}

	//Check if we need to adjust move_vec or object orientation because of gravity changes
	btVector3 vec2 = btVector3(0,0,-1).cross(phys_world->getGravity());
	float obj_adj_rot = btVector3(1,0,0).angle(vec2);
	if(obj_adj_rot > 0){
		//we want to know the whole rotation around the z axis (not from 0,pi)
		float dot = btVector3(0,1,0).dot(vec2);
		if( dot < 0 ){
			obj_adj_rot = - obj_adj_rot;
		}
		btQuaternion quat;
		quat.setEuler(0, 0, obj_adj_rot); //or quat.setEulerZYX depending on the ordering you want
		//Make the phys body face "up"
		phys_body->getWorldTransform().setRotation(quat);
	}

	if(!moving && can_jump()){
		phys_body->setFriction(2.5f);
	} else {
		phys_body->setFriction(0.5f);
	}
	if(moving && !jumping){
		adj_move_vec = old_move_vec.lerp(move_vec, cur_move_speed);
			
		if(cur_move_speed < 1.0f && move_timer.delta_s() > 0.02){
			move_timer.start();
			cur_move_speed += 0.05f;
		}

		//NPC know where they want to go (left/right) in the game world
		if(!npc){
			adj_move_vec = adj_move_vec.x() * vec2.normalized();
		}
		btVector3 nor_grav =  phys_world->getGravity().normalized();

        btVector3 grav_vec =  phys_body->getLinearVelocity().dot(nor_grav) * nor_grav;

        btVector3 surf_nor;

		if(can_jump(surf_nor)){
			btVector3 perp_floor = surf_nor.cross(btVector3(0,0,-1));
			float angle = (perp_floor * perp_floor.dot(adj_move_vec)).angle(phys_world->getGravity());
			if(angle > M_PI_2){
				phys_body->setLinearVelocity(adj_move_vec * 3.5f );
			} else {
				//If we are moving along a downward slope don't "step out" into the air.
				phys_body->setLinearVelocity( perp_floor * perp_floor.dot(adj_move_vec) * 3.5f );
			}
		} else {
			float vec_l = btFabs(( phys_body->getLinearVelocity() - grav_vec).dot(adj_move_vec)) - 2.0f;
			if(vec_l < 0.0f){
			phys_body->setLinearVelocity( phys_body->getLinearVelocity() + adj_move_vec * 0.25f );
			}
		}
	}
	if(jumping && jump_timer.delta_s() > 0.2){
		cur_move_speed = 0;
		jumping = false;
		jump_timer.stop();
	}
}

void GameObject::attack(btVector3 dir, int dmg){
 	btTransform pos_to, pos_from;
	btSphereShape sphere(0.1f);
	phys_body->getMotionState()->getWorldTransform(pos_from);
	//Reset rotation so that we only get the origin coords
	pos_from.setRotation(btQuaternion(0,0,0));

    btVector3 extend_dir = 6*(dir - pos_from.getOrigin()).normalized();

	pos_to.setIdentity();
	pos_to.setOrigin(extend_dir + pos_from.getOrigin());

    SDL_Point shoot = {pos_from.getOrigin().getX()*80, pos_from.getOrigin().getY()*80};
	shoot_vec.push_back(shoot);

	ClosestNotMeSweep cb( phys_body, pos_from.getOrigin(), pos_to.getOrigin() );
	cb.m_collisionFilterGroup = objCollidesWith;

	phys_world->convexSweepTest(&sphere, pos_from, pos_to, cb);
	if (cb.hasHit())
	{
		GameObject *obj = static_cast<GameObject*>(cb.m_hitCollisionObject->getUserPointer());
		//Is this a non static object?
		if(obj != NULL){
			obj->apply_dmg(dmg);
			if(!obj->get_controllable() || obj->get_dead()){
				float hit_force = 100.0f;
				btVector3 force_dir = dir - pos_from.getOrigin();
				btVector3 rel_pos = cb.m_hitPointWorld - obj->get_body()->getCenterOfMassPosition();
				obj->get_body()->applyImpulse(force_dir.normalized() * hit_force, rel_pos);
			}
		}
		shoot = { cb.m_hitPointWorld.getX() * 80, cb.m_hitPointWorld.getY() * 80 };
		shoot_vec.push_back(shoot);
	} else {
		shoot = { pos_to.getOrigin().getX() * 80, pos_to.getOrigin().getY() * 80 };
		shoot_vec.push_back(shoot);
	}
}

void GameObject::apply_dmg(int dmg){
	if(godmode){
		return;
	}
	health -= dmg;
	if( !npc ){
		//have some revovery time after a hit
		godmode = true;
		hit_timer.start();
	}
	if(health <= 0){
		dead = true;
        //Switch collision group to dead (need to init a new phys body)
		btTransform trans;
        phys_body->getMotionState()->getWorldTransform(trans);
        spawn_x = trans.getOrigin().getX();
		spawn_y = trans.getOrigin().getY();
		btVector3 rot_vec, velo_vec;
		QuaternionToEulerXYZ(trans.getRotation(),rot_vec);
		spawn_rot = rot_vec.getZ();
		velo_vec = phys_body->getLinearVelocity();
		init();
		phys_body->setLinearVelocity(velo_vec);
		//Fall over if dead
		phys_body->setAngularFactor(btVector3(0,0,1));
	}
}

bool GameObject::get_controllable(){
 	return controllable;
}

bool GameObject::get_npc(){
	return npc;
}

bool GameObject::get_dead(){
	return dead;
}

int GameObject::get_hp(){
	return health;
}

int GameObject::get_max_hp(){
	return 10;
}

void GameObject::apply_spawn_offset(int x, int y){
	spawn_x += x/80.0f;
	spawn_y += y/80.0f;
}

void GameObject::npc_think(){
	if(!player_obj){
		return;
	}

	btTransform pos_to, pos_from;
	btSphereShape sphere(0.1f);
	phys_body->getMotionState()->getWorldTransform(pos_from);
	//Reset rotation so that we only get the origin coords
	pos_from.setRotation(btQuaternion(0,0,0));
    player_obj->get_body()->getMotionState()->getWorldTransform(pos_to);
	pos_to.setRotation(btQuaternion(0,0,0));

	ClosestNotMeSweep cb( phys_body, pos_from.getOrigin(), pos_to.getOrigin() );
	cb.m_collisionFilterGroup = playerCollidesWith;
	cb.m_collisionFilterMask = COL_PLAYER | COL_WALL | COL_OBJ;

    btVector3 move_dir = pos_to.getOrigin() - pos_from.getOrigin();

	phys_world->convexSweepTest(&sphere, pos_from, pos_to, cb);
	if (cb.hasHit())
	{
		GameObject *obj = static_cast<GameObject*>(cb.m_hitCollisionObject->getUserPointer());
		if(obj == player_obj ){
			if( move_dir.length() > 0.7 ){
				moving = true;
				move_timer.start();
				old_move_vec = move_vec;
				btVector3 dir_norm = move_dir.normalized();
				move_vec = dir_norm;
				if(dir_norm.dot( phys_world->getGravity().normalized() ) < -0.5){
					jump();
				}
			} else {
				moving = false;
				move_timer.stop();
				old_move_vec = move_vec;
				move_vec.setZero();
			}
			if( !obj->get_dead() && move_dir.length() < 0.7 ){
				attack(pos_to.getOrigin() ,1);	
			}
		} else {
			moving = false;
			move_timer.stop();
			old_move_vec = move_vec;
			move_vec.setZero();
		}
	}
}

void GameObject::QuaternionToEulerXYZ(const btQuaternion &quat,btVector3 &euler)
{
	float w=quat.getW();   float x=quat.getX();   float y=quat.getY();   float z=quat.getZ();
	double sqw = w*w; double sqx = x*x; double sqy = y*y; double sqz = z*z;
	euler.setZ((atan2(2.0 * (x*y + z*w),(sqx - sqy - sqz + sqw))));
	euler.setX((atan2(2.0 * (y*z + x*w),(-sqx - sqy + sqz + sqw))));
	euler.setY((asin(-2.0 * (x*z - y*w))));
}

void GameObject::render_obj(int off_x, int off_y){
	SDL_Rect dest;
	btTransform trans;
	phys_body->getMotionState()->getWorldTransform(trans);

	btVector3 rot_vec;
	QuaternionToEulerXYZ(trans.getRotation(),rot_vec);
	float rot = (rot_vec.getZ() / M_PI) * 180.0f;

	dest.x = off_x - tile.rect.w/2 + trans.getOrigin().getX() * 80;
	dest.y = off_y - tile.rect.h/2 + trans.getOrigin().getY() * 80;
	dest.w = tile.rect.w;
	dest.h = tile.rect.h;

	SDL_RenderCopyEx(renderer, tile.texture, &tile.rect, &dest, rot, NULL, SDL_FLIP_NONE);
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	for(int i = 0; i < shoot_vec.size(); i += 2){
		SDL_RenderDrawLine(renderer, off_x + shoot_vec[i].x,  off_y + shoot_vec[i].y, off_x + shoot_vec[i+1].x,  off_y + shoot_vec[i+1].y);
	}
	shoot_vec.clear();
}
