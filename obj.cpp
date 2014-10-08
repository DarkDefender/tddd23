#include <SDL2/SDL.h>
#include <btBulletDynamicsCommon.h>
#include <unordered_map>
#include "sdl_h_func.h"
#include "obj.h"
#include <math.h>  
#include <iostream>
#include "timer.h"

using namespace std;

unordered_map<string,btCollisionShape*> GameObject::obj_coll_shape;
btDiscreteDynamicsWorld* GameObject::phys_world = NULL;
SDL_Renderer* GameObject::renderer = NULL;

GameObject::GameObject( string body_type, string tile_set, uint8_t health, uint32_t x, uint32_t y, bool is_controllable ){
	inited = false;
	phys_body = NULL;
	moving = false;
	jumping = false;
	controllable = is_controllable;
	cur_move_speed = 0;
	jump_vec.setZero();
	move_vec.setZero();
	old_move_vec.setZero();
	adj_move_vec.setZero();
	spawn_x = x;
	spawn_y = y;
	if(obj_coll_shape.count(body_type) == 0){
		//TODO add proper shape creation based on string
		if(body_type == "box"){
			obj_coll_shape[body_type] = new btBoxShape(btVector3(0.5f,0.5f,0.5f));
		} else {
		obj_coll_shape[body_type] = new btSphereShape(0.5f);
		}
	}
	body_shape = obj_coll_shape[body_type];
	obj_name = tile_set;

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
	btDefaultMotionState* MotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(spawn_x, spawn_y, 0)));
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

	//TODO only for certain situations!
    phys_body->setActivationState(DISABLE_DEACTIVATION);

    //Prevent tunneling
    //setup motion clamping so no tunneling occurs
	//phys_body->setCcdMotionThreshold(1);
	//phys_body->setCcdSweptSphereRadius(0.2f);

	phys_world->addRigidBody(phys_body);

	texture = loadTexture(obj_name, renderer);
}

void GameObject::clean_up(){
	phys_world->removeRigidBody(phys_body);
	delete phys_body->getMotionState();
	delete phys_body; 
}

GameObject::~GameObject(){
	SDL_DestroyTexture( texture );
	clean_up();
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
	btSphereShape sphere(0.4f);
	phys_body->getMotionState()->getWorldTransform(pos_from);
	pos_to.setIdentity();
	pos_to.setOrigin(0.15f * nor_grav + pos_from.getOrigin());

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
	btSphereShape sphere(0.4f);
	phys_body->getMotionState()->getWorldTransform(pos_from);
	pos_to.setIdentity();
	pos_to.setOrigin(0.15f * nor_grav + pos_from.getOrigin());

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
	btSphereShape sphere(0.4f);
	btTransform pos_to, pos_from;
	phys_body->getMotionState()->getWorldTransform(pos_from);
	pos_to.setIdentity();
	
	//right wall
	pos_to.setOrigin(0.15f * phys_world->getGravity().cross(btVector3(0,0,1)).normalized() + pos_from.getOrigin());

	ClosestNotMeSweep cb( phys_body, pos_from.getOrigin(), pos_to.getOrigin() );

	phys_world->convexSweepTest(&sphere, pos_from, pos_to, cb);

	if (cb.hasHit())
	{
		nor_vec = cb.m_hitNormalWorld.rotate(btVector3(0,0,1), M_PI_4).normalized();
		return true;
	} 
	//left wall
	pos_to.setOrigin(0.15f * phys_world->getGravity().cross(btVector3(0,0,-1)).normalized() + pos_from.getOrigin());

	ClosestNotMeSweep cb2( phys_body, pos_from.getOrigin(), pos_to.getOrigin() );

	phys_world->convexSweepTest(&sphere, pos_from, pos_to, cb2);

	if (cb2.hasHit()) {
		nor_vec = cb2.m_hitNormalWorld.rotate(btVector3(0,0,1), -M_PI_4).normalized();
		return true;
	}
	return false;
}

bool GameObject::can_jump_static(){
	btSphereShape sphere(0.4f);
	btTransform pos_to, pos_from;
	phys_body->getMotionState()->getWorldTransform(pos_from);
	pos_to.setIdentity();
	pos_to.setOrigin(0.15f * phys_world->getGravity().normalized() + pos_from.getOrigin());

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
		if(phys_body->getLinearVelocity().dot(jump_vec) > 0.1f){ 
			phys_body->applyCentralImpulse(-jump_vec * (3.0f/delta) );
		}
		jump_timer.stop();
	}
}

void GameObject::update(){
	if(!controllable){
   		return;
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

		//Check if we need to adjust move_vec because of gravity changes
		btVector3 vec2 = btVector3(0,0,-1).cross(phys_world->getGravity());

  		adj_move_vec = adj_move_vec.x() * vec2.normalized();
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

	dest.x = off_x - 40 + trans.getOrigin().getX() * 80;
	dest.y = off_y - 40 + trans.getOrigin().getY() * 80;
	dest.w = 80;
	dest.h = 80;

	SDL_RenderCopyEx(renderer, texture, NULL, &dest, rot, NULL, SDL_FLIP_NONE);
}
