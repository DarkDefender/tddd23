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
int off_x = 0;
int off_y = 0;

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

    level->render_layers(renderer, off_x, off_y);

    for (list<Text_box*>::iterator it = game_object_list.begin(); it != game_object_list.end(); it++){
    	(*it)->render_text();
    }

    SDL_RenderPresent(renderer);
}

btTriangleMesh* create_terrain(vector<vector<SDL_Point>> zone_coll){
	btTriangleMesh* trimesh = new btTriangleMesh();
	for(unsigned int p = 0; p < zone_coll.size(); p++){
		for(unsigned int j = 0; j < zone_coll[p].size() - 1; j++){
			//convert the 2d line to a 3d plane
			btScalar p1_x = zone_coll[p][j].x;
			btScalar p1_y = zone_coll[p][j].y;
			btScalar p2_x = zone_coll[p][j+1].x;
			btScalar p2_y = zone_coll[p][j+1].y;

			p1_x = p1_x / 40.0f;
			p1_y = p1_y / 40.0f;
			p2_x = p2_x / 40.0f;
			p2_y = p2_y / 40.0f;

			trimesh->addTriangle( btVector3(p1_x, p1_y , -1),
								  btVector3(p1_x, p1_y , 1),
								  btVector3(p2_x, p2_y , 1));

			trimesh->addTriangle( btVector3(p2_x, p2_y , 1),
								  btVector3(p2_x, p2_y , -1),
								  btVector3(p1_x, p1_y , -1));
		}
	}
	return trimesh;
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
    
	//Default gravity is -10, but here the the game world has the y axis inverted to grav is +10
	btVector3 grav_vec = btVector3(0, 10, 0);
	dynamicsWorld->setGravity(grav_vec);

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
		new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(6, 10, 0)));
	btScalar mass = 10;
	btVector3 fallInertia(0, 0, 0);
	fallShape->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
	btRigidBody* fallRigidBody = new btRigidBody(fallRigidBodyCI);

	// Limit movement to the x,y axis. And limit rotation around the z axis
	fallRigidBody->setLinearFactor(btVector3(1,1,0));
	fallRigidBody->setAngularFactor(btVector3(0,0,1));

	dynamicsWorld->addRigidBody(fallRigidBody);

    // Setup the world shape
	// TODO init the whole world
    btTriangleMesh* trimesh = create_terrain(level->get_coll_vec());

	btCollisionShape *mTriMeshShape = new btBvhTriangleMeshShape(trimesh,true);

	btDefaultMotionState* levelMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));

	btRigidBody::btRigidBodyConstructionInfo
		levelRigidBodyCI(0, levelMotionState, mTriMeshShape, btVector3(0, 0, 0));
	btRigidBody* levelRigidBody = new btRigidBody(levelRigidBodyCI);
	dynamicsWorld->addRigidBody(levelRigidBody);

    // Nudge the circle
	btVector3 up = btVector3(0, -20, 0);
	btVector3 down = btVector3(0, 20, 0);
	btVector3 left = btVector3(-20, 0, 0);
	btVector3 right = btVector3(20, 20, 0);
	//fallRigidBody->applyCentralImpulse(force);

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
				case SDL_MOUSEBUTTONDOWN:
				    /*
					off_x = - event.button.x;
					off_y = - event.button.y;
					*/
					break;
				case SDL_KEYDOWN:
					if(event.key.repeat){
						// We don't want to handle key repeats
						break;
					}
					fallRigidBody->activate(true);
					switch (event.key.keysym.sym)
					{
						case SDLK_LEFT:  fallRigidBody->applyCentralImpulse(left); break;
						case SDLK_RIGHT: fallRigidBody->applyCentralImpulse(right); break;
						case SDLK_UP:    fallRigidBody->applyCentralImpulse(up); break;
						case SDLK_DOWN:  fallRigidBody->applyCentralImpulse(down); break;
						case SDLK_SPACE:
										 grav_vec = grav_vec.rotate(btVector3(0,0,1),0.2);
										 dynamicsWorld->setGravity(grav_vec);
										 break;
					}
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

        off_x = -trans.getOrigin().getX() * 20;
		off_y = -trans.getOrigin().getY() * 20;

		b2->set_pos(off_x + trans.getOrigin().getX() * 40, off_y + trans.getOrigin().getY() * 40);
		b2->new_text("x: " + to_string(trans.getOrigin().getX()) + " y: " + to_string(trans.getOrigin().getY()));
	}

	//BULLET clean

	dynamicsWorld->removeRigidBody(fallRigidBody);
	delete fallRigidBody->getMotionState();
	delete fallRigidBody;

	dynamicsWorld->removeRigidBody(groundRigidBody);
	delete groundRigidBody->getMotionState();
	delete groundRigidBody;

	dynamicsWorld->removeRigidBody(levelRigidBody);
	delete levelRigidBody->getMotionState();
	delete levelRigidBody;

	delete fallShape;

	delete groundShape;

	delete mTriMeshShape;

	delete trimesh;
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
