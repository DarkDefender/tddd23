#include <SDL2/SDL.h>
#include <btBulletDynamicsCommon.h>
#include <unordered_map>
#include "sdl_h_func.h"
#include "obj.h"
#include <math.h>  

unordered_map<string,btCollisionShape*> GameObject::obj_coll_shape;
btDiscreteDynamicsWorld* GameObject::phys_world = NULL;
SDL_Renderer* GameObject::renderer = NULL;

GameObject::GameObject( string body_type, string tile_set, uint8_t health, uint32_t x, uint32_t y ){
	inited = false;
	spawn_x = x;
	spawn_y = y;
	if(obj_coll_shape.count(body_type) == 0){
		//TODO add proper shape creation based on string
		if(body_type == "box"){
			obj_coll_shape[body_type] = new btBoxShape(btVector3(1,1,1));
		} else {
		obj_coll_shape[body_type] = new btSphereShape(1);
		}
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
	float rot = (rot_vec.getZ() / 3.14) * 180.0f;

	dest.x = off_x - 40 + trans.getOrigin().getX() * 40;
	dest.y = off_y - 40 + trans.getOrigin().getY() * 40;
	dest.w = 80;
	dest.h = 80;

	SDL_RenderCopyEx(renderer, texture, NULL, &dest, rot, NULL, SDL_FLIP_NONE);
}
