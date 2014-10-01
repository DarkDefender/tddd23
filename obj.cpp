#include <SDL2/SDL.h>
#include <btBulletDynamicsCommon.h>
#include <unordered_map>
#include "sdl_h_func.h"
#include "obj.h"

unordered_map<string,btCollisionShape*> GameObject::obj_coll_shape;
btDiscreteDynamicsWorld* GameObject::phys_world = NULL;
SDL_Renderer* GameObject::renderer = NULL;

GameObject::GameObject( string body_type, string tile_set, uint8_t health, uint32_t x, uint32_t y ){
	inited = false;
	spawn_x = x;
	spawn_y = y;
	if(obj_coll_shape.count(body_type) == 0){
		//TODO add proper shape creation based on string
		obj_coll_shape[body_type] = new btSphereShape(1);
	}
	body_shape = obj_coll_shape[body_type];
	obj_name = tile_set;

	if(renderer != NULL && phys_body != NULL){
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
	phys_body->setAngularFactor(btVector3(0,0,1));

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

void GameObject::render_obj(){
	SDL_Rect dest;
	btTransform trans;
	phys_body->getMotionState()->getWorldTransform(trans);

	float angle = trans.getRotation().getAngle();

	dest.x = trans.getOrigin().getX() * 40;
	dest.y = trans.getOrigin().getY() * 40;
	dest.w = 40;
	dest.h = 40;
	SDL_RenderCopyEx(renderer, texture, NULL, &dest, angle, NULL, SDL_FLIP_NONE);
}
