#include <iostream>
#include <cstring> 
#include <list>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include "gui/text_box.h"
#include "timer.h"

#include "level.h"

#include <btBulletDynamicsCommon.h>

const int WIDTH = 640;
const int HEIGHT = 480;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;

using namespace std;

void cleanup(int exitcode){
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	exit(exitcode);
}

void update_screen(SDL_Renderer *renderer, list<Text_box*> game_object_list, LevelZone *level){
    /* Clear the background to background color */
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    level->render_layers(renderer);

    for (list<Text_box*>::iterator it = game_object_list.begin(); it != game_object_list.end(); it++){
    	(*it)->render_text();
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]){
	
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Event event;

	cout << "Hello World!\n";

	// Init the TTF lib
	if ( TTF_Init() < 0 ) {
		cerr << "Couldn't initialize TTF: " << SDL_GetError() << endl;
		SDL_Quit();
		return 2;
	}
    //Initialize PNG loading 
	int imgFlags = IMG_INIT_PNG; 
	if( !( IMG_Init( imgFlags ) & imgFlags ) ) {
		cerr << "SDL_image could not initialize! SDL_image Error:" << IMG_GetError() << endl;
        TTF_Quit();
		SDL_Quit();
		return 2; 
	}
	// Create a window
	if (SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer) < 0) {
		cerr << "SDL_CreateWindowAndRenderer() failed: " << SDL_GetError() << endl;
		cleanup(2);
	}

	//TODO use scaling
    //SDL_RenderSetScale(renderer,1,1);

	// Create test text objects
	// TODO remeber to free this up later
	Text_box *b1 = new Text_box(10,10,110,50,"../res/fonts/PROBE_10PX_TTF.ttf");
	// Only need to set renderer one time (shared between objects)
	// TODO psf generation will fail if renderer is not set!
	b1->set_renderer(renderer);
	Text_box *b2 = new Text_box(10,40,100,50,"../res/fonts/Tewi-normal-11.psf");

	b1->new_text("Hipp. Happ, hopp!");
    b2->new_text("Hipp. Happ, popp!");

	list<Text_box*> obj_list;
	obj_list.push_back(b1);
	obj_list.push_back(b2);

    //Load level
    LevelZone *level = new LevelZone("untitled.tmx", renderer ); 

	//---- BULLET INIT 
    // Build the broadphase
    btBroadphaseInterface* broadphase = new btDbvtBroadphase();

    // Set up the collision configuration and dispatcher
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

    // The actual physics solver
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

    // The world.
    btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, -10, 0));

    //---- END BULLET INIT

	// Setup bullet shapes

	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);

	btCollisionShape* fallShape = new btSphereShape(1);

	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));

	btRigidBody::btRigidBodyConstructionInfo
		groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
	btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
	dynamicsWorld->addRigidBody(groundRigidBody);

	btDefaultMotionState* fallMotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 10, 0)));
	btScalar mass = 10;
	btVector3 fallInertia(0, 0, 0);
	fallShape->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
	btRigidBody* fallRigidBody = new btRigidBody(fallRigidBodyCI);
	dynamicsWorld->addRigidBody(fallRigidBody);

	btVector3 force = btVector3(0, 20, 0);
	fallRigidBody->applyCentralImpulse(force);

	//Setup timer
    Timer fps_timer;
	Timer fps_cap_timer;
	bool done = false;
    int counted_frames = 0;
	fps_timer.start();

	while ( ! done ) {

        //(Re)start the fps cap timer
		fps_cap_timer.start();

		while( SDL_PollEvent( &event ) != 0 ){  
			switch (event.type) {
				/*
				   case SDL_MOUSEBUTTONDOWN:
				   scene.messageRect.x = event.button.x - text->w/2;
				   scene.messageRect.y = event.button.y - text->h/2;
				   scene.messageRect.w = text->w;
				   scene.messageRect.h = text->h;
				   draw_scene(renderer, &scene);
				   break;
				   */
				case SDL_KEYDOWN:
					fallRigidBody->activate(true);
					fallRigidBody->applyCentralImpulse(force);
					break;
				case SDL_QUIT:
					done = 1;
					break;
				default:
					break;
			}
		}
		//Calculate and correct fps
		float avg_fps = counted_frames / fps_timer.delta_s();
		if( avg_fps > 2000000 ){
			avg_fps = 0;
		}

		update_screen(renderer, obj_list, level);
        ++counted_frames;

		//Cap framerate
		int frame_ticks = fps_cap_timer.getTicks();
		if( frame_ticks < SCREEN_TICKS_PER_FRAME - frame_ticks){
			SDL_Delay( SCREEN_TICKS_PER_FRAME - frame_ticks );
		}
        //update physics
		dynamicsWorld->stepSimulation(fps_cap_timer.delta_s());

		btTransform trans;
		fallRigidBody->getMotionState()->getWorldTransform(trans);

		b2->set_pos(10 + trans.getOrigin().getX() * 40,  40 + trans.getOrigin().getY() * 40);
		b2->new_text("x: " + to_string(trans.getOrigin().getX()) + " y: " + to_string(trans.getOrigin().getY()));
	}

	//BULLET clean

	dynamicsWorld->removeRigidBody(fallRigidBody);
	delete fallRigidBody->getMotionState();
	delete fallRigidBody;

	dynamicsWorld->removeRigidBody(groundRigidBody);
	delete groundRigidBody->getMotionState();
	delete groundRigidBody;


	delete fallShape;

	delete groundShape;
    // Clean up behind ourselves like good little programmers
    delete dynamicsWorld;
    delete solver;
    delete dispatcher;
    delete collisionConfiguration;
    delete broadphase;

	// TODO clean up all loaded fonts!
	//TTF_CloseFont(font);  
	SDL_Quit();

	return 0;
}
