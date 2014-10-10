#include "level.h"
#include "sdl_h_func.h"
#include <pugixml.hpp>
#include <SDL2/SDL.h>

#include <iostream>
#include <cstring>
#include <vector>
#include <unordered_map>

#include "obj.h"
#include <list>
#include <btBulletDynamicsCommon.h> 

using namespace std; 

const int WIDTH = 640;
const int HEIGHT = 480;

unordered_map<string,pugi::xml_document> LevelZone::zones;
unordered_map<string,SDL_Texture *> LevelZone::images;

Level::Level(string level_file, SDL_Renderer *renderer){
    render_rot = 0;
	prev_cam_vec.resize(15);
	cam_vec_id = 0;
	world_scale = 80.0f;
	focus_obj = NULL;
	render_offset = {0,0};
	cur_tile = {-1,-1};
	level_texture = NULL;
	rotate_world = false;


	pugi::xml_document level;
	
	//TODO handle faliures
	pugi::xml_parse_result result = level.load_file(level_file.c_str());
	int level_w, level_h;

    pugi::xml_node map = level.child("map");

    tile_w = map.attribute("tilewidth").as_int();
	tile_h = map.attribute("tileheight").as_int();  
	level_w = map.attribute("width").as_int();
	level_h = map.attribute("height").as_int();
	//load in the level tile numbers
	
	int i = 1;
	vector<LevelZone*> tile_vec;
	for( pugi::xml_node tiles = map.child("layer").child("data").first_child(); tiles; tiles = tiles.next_sibling()) {
        if( i == level_w){
            l_zone_tiles.push_back(tile_vec);
			tile_vec.clear();
            i = 1;
            continue;
        } else {
			string tile_no(tiles.attribute("gid").value());
			if(tile_no == "0"){
				tile_vec.push_back(NULL);
			} else {
            tile_vec.push_back(new LevelZone(tile_no + ".tmx", renderer));
			}
        }
        i++;
    }
	//TODO get filenames from gids to load level tiles

    /*
    //Load level
	vector<LevelZone*> lvl_vec;
    lvl_vec.push_back(new LevelZone("untitled.tmx", renderer ));
	l_zone_tiles.push_back(lvl_vec);
    */

	//Setup bullet world
	setup_bullet_world();

	//Create a game object
	//TODO remember to free/delete this later
	GameObject *player = new GameObject("circle", "circle.png", 10, 6, 10, true);
	//Only need to set renderer and phys world once.
	player->set_renderer(renderer);
	player->set_phys_world(dynamicsWorld);
	//Only need to call init for the first object created after renderer and phys world has been set
	//player->init();

	obj_list.push_back(player);

	get_lvl_objs();

	focus_obj = player;
}

Level::~Level(){
	if(level_texture != NULL){
		SDL_DestroyTexture( level_texture );
	}

    del_bullet_world();

	//TODO clean up object list & level zones
}

void Level::setup_bullet_world(){
	//---- BULLET INIT 
    // Build the broadphase
    broadphase = new btDbvtBroadphase();

    // Set up the collision configuration and dispatcher
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);

    // The actual physics solver
    solver = new btSequentialImpulseConstraintSolver;

    // The world.
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    
	//Default gravity is -10, but here the the game world has the y axis inverted to grav is +10
	grav_vec = btVector3(0, 10, 0);
	dynamicsWorld->setGravity(grav_vec);

    //---- END BULLET INIT
	
	create_terrain();
	mTriMeshShape = new btBvhTriangleMeshShape(level_trimesh,true);
	btDefaultMotionState* levelMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
	
	btRigidBody::btRigidBodyConstructionInfo
		levelRigidBodyCI(0, levelMotionState, mTriMeshShape, btVector3(0, 0, 0));
	levelRigidBody = new btRigidBody(levelRigidBodyCI);
	dynamicsWorld->addRigidBody(levelRigidBody);
}

void Level::del_bullet_world(){
	
	dynamicsWorld->removeRigidBody(levelRigidBody);
	delete levelRigidBody->getMotionState();
	delete levelRigidBody;

	delete mTriMeshShape;

	delete level_trimesh; 

    // Clean up behind ourselves like good little programmers
    delete dynamicsWorld;
    delete solver;
    delete dispatcher;
    delete collisionConfiguration;
    delete broadphase;
}

void Level::get_lvl_objs(){
	for(unsigned int i = 0; i < l_zone_tiles.size(); i++){
		for(unsigned int q = 0; q < l_zone_tiles[i].size(); q++){

			if(l_zone_tiles[i][q] == NULL){
				continue;
			}
			list<GameObject*> *zone_objs = l_zone_tiles[i][q]->get_objs();
			obj_list.splice(obj_list.end(), *zone_objs);
			zone_objs->clear();
		}
	}
	//Init all objs
	for (auto it = obj_list.begin(); it != obj_list.end(); it++){
		(*it)->init();
	}
}

void Level::create_terrain(){
	level_trimesh = new btTriangleMesh();
	for(unsigned int i = 0; i < l_zone_tiles.size(); i++){
		for(unsigned int q = 0; q < l_zone_tiles[i].size(); q++){

            if(l_zone_tiles[i][q] == NULL){
				continue;
			}

			vector<vector<SDL_Point>> zone_coll = l_zone_tiles[i][q]->get_coll_vec();

			for(unsigned int p = 0; p < zone_coll.size(); p++){
				for(unsigned int j = 0; j < zone_coll[p].size() - 1; j++){
					//convert the 2d line to a 3d plane, add tile offset
					btScalar p1_x = q * tile_h * 10 + zone_coll[p][j].x;
					btScalar p1_y = i * tile_w * 10 + zone_coll[p][j].y;
					btScalar p2_x = q * tile_h * 10 + zone_coll[p][j+1].x;
					btScalar p2_y = i * tile_w * 10 + zone_coll[p][j+1].y;

					p1_x /= world_scale;
					p1_y /= world_scale;
					p2_x /= world_scale;
					p2_y /= world_scale;

					level_trimesh->addTriangle( btVector3(p1_x, p1_y , -1),
							btVector3(p1_x, p1_y , 1),
							btVector3(p2_x, p2_y , 1));

					level_trimesh->addTriangle( btVector3(p2_x, p2_y , 1),
							btVector3(p2_x, p2_y , -1),
							btVector3(p1_x, p1_y , -1));
				}
			}
		}
	}
}

void Level::update_offset(){
	SDL_Point focus_point = focus_obj->get_pos(world_scale);

	if(cur_tile.x == -1){
		//We want to have the camera begin were we are spawning
		for(unsigned int i = 0; i < prev_cam_vec.size() ; i++){
			prev_cam_vec[i] = {-focus_point.x + WIDTH/2,
							   -focus_point.y + HEIGHT/2};
		}
		return;
	}

	prev_cam_vec[cam_vec_id] = {-focus_point.x + WIDTH/2,
		-focus_point.y + HEIGHT/2};

	cam_vec_id++;
	if( cam_vec_id >= prev_cam_vec.size() ){
		cam_vec_id = 0;
	}

	render_offset = {0,0};

	for(unsigned int i = 0; i < prev_cam_vec.size() ; i++){
		render_offset.x += prev_cam_vec[i].x;
		render_offset.y += prev_cam_vec[i].y;
	}

	render_offset.x /= 15;
	render_offset.y /= 15;
}

SDL_Point Level::get_render_offset(){
	return render_offset;
}

void Level::update_tile_index(){
	//where are we?
	cur_tile.x = abs(render_offset.x) / (tile_w * 10);
	cur_tile.y = abs(render_offset.y) / (tile_h * 10);
	//cerr << cur_tile.x << " y: " << cur_tile.y << endl;

}

void Level::draw_level(SDL_Renderer *renderer){
	if( focus_obj == NULL){
		cerr << "No focus object!" << endl;
		return;
	}
	update_offset();
	update_tile_index();

	if ( level_texture == NULL ){
		//TODO calc the exact needed size
		level_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 800, 800);
		SDL_SetTextureBlendMode(level_texture, SDL_BLENDMODE_BLEND);
	}

	SDL_SetRenderTarget(renderer, level_texture);
	/* Clear the background to background color */
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(renderer);

	for(int i = 0; i < 2; i++){
		for(int j = 0; j < 2; j++){
			LevelZone* zone = l_zone_tiles.at(cur_tile.y + i).at(cur_tile.x + j);
			if(zone != NULL){
				zone->render_layers(renderer, (cur_tile.x + j) * tile_w * 10 + render_offset.x+80, (cur_tile.y + i) * tile_h * 10 + render_offset.y+160);
			}
		}
	}

	for (list<GameObject*>::iterator it = obj_list.begin(); it != obj_list.end(); it++){
		(*it)->render_obj(render_offset.x+80, render_offset.y+160);
		(*it)->update();
	}
	SDL_SetRenderTarget(renderer, NULL);
	SDL_Rect dest = {-80,-160,800,800};
	SDL_RenderCopyEx(renderer, level_texture, NULL, &dest, render_rot, NULL, SDL_FLIP_NONE);
}

void Level::toggle_rotate_world(){
	rotate_world = !rotate_world;
}

void Level::update(float delta_s){
	//Rotate the world 4 degree per sec
	if(rotate_world){
		grav_vec = grav_vec.rotate(btVector3(0,0,1),0.07*delta_s);

		render_rot -= 4 * delta_s;
		dynamicsWorld->setGravity(grav_vec);
	}
	dynamicsWorld->stepSimulation(delta_s);
}

GameObject* Level::get_player(){
	return focus_obj;
}

void LevelZone::parse_collison_obj( pugi::xml_node node, int tile_id ){
	for (pugi::xml_node object = node.child("object"); object; object = object.next_sibling("object")) {
		
		int origin_x, origin_y;
		SDL_Point coord;
		vector<SDL_Point> coord_vec;
		origin_x = object.attribute("x").as_int();
		origin_y = object.attribute("y").as_int();
		pugi::xml_node coll_shape = object.first_child();
		string node_name( coll_shape.name() );
		//Is this a polyline?
		if( node_name == "polyline") {
			char *pch;
			int i = 0;
			string str( coll_shape.attribute("points").value() );
			char chars[str.size() +1];

			strcpy( chars, str.c_str() );
			//split the string at space and comma char
			pch = strtok( chars, ", " );
			while (pch != NULL) {
				if ( i % 2 ){
					coord.y = stoi(pch) + origin_y;
					coord_vec.push_back(coord);
				} else {
					coord.x = stoi(pch) + origin_x;
				}
				pch = strtok (NULL, ", ");
				i++;
			}
		} else {
			cout << "Unknown coll_shape: " << node_name << endl;
		}

		//Is this a zone coll_shape or a tile coll_shape?
		if(tile_id == 0){
            //This is a zone shape
            zone_coll.push_back(coord_vec);
		} else {
            tile_tex[tile_id - 1].coll.push_back(coord_vec);
		}
	}
}

LevelZone::LevelZone(string level_zone_file, SDL_Renderer *renderer){
	zone_name = level_zone_file;
	//Only have one copy of the same zone loaded
	if (zones.count(level_zone_file) == 0){
		//TODO handle faliures
		pugi::xml_parse_result result = zones[level_zone_file].load_file(level_zone_file.c_str());
	}
	pugi::xml_node map = zones[zone_name].child("map");

	zone_tile_w = map.attribute("tilewidth").as_int();
	zone_tile_h = map.attribute("tileheight").as_int();
	zone_w = map.attribute("width").as_int();
	zone_h = map.attribute("height").as_int();

	// Load layer data
	for (pugi::xml_node node = map.child("layer"); node; node = node.next_sibling("layer")) {
		//load in the level tile numbers
		unsigned int i = 1;
		vector<vector<unsigned int>> zone_tiles;
		vector<unsigned int> tile_vec;
		for( pugi::xml_node tiles = node.child("data").first_child(); tiles; tiles = tiles.next_sibling()) {
			if( i == zone_w){
				zone_tiles.push_back(tile_vec);
				tile_vec.clear();
				i = 1;
				continue;
			} else {
				tile_vec.push_back(tiles.attribute("gid").as_int());
			}
			i++;
		}
		level_tiles.push_back(zone_tiles);
	}

    // Load all images required for this zone
	for (pugi::xml_node node = map.child("tileset"); node; node = node.next_sibling("tileset")) {
		int tile_w, tile_h, first_id;
		tile_w = node.attribute("tilewidth").as_int();
		tile_h = node.attribute("tileheight").as_int();
		first_id = node.attribute("firstgid").as_int();
		for( pugi::xml_node node2 = node.first_child(); node2; node2 = node2.next_sibling()) {
			Tile cur_tile;
			cur_tile.rect.x = 0;
			cur_tile.rect.y = 0;
			string img_path;
            string node_name( node2.name() );
			// If this is a tile node, we must check if image node exists
			if( node_name == "tile" && node2.child("image") ){
				img_path = node2.child("image").attribute("source").value();
				cur_tile.rect.w = node2.child("image").attribute("width").as_int();
				cur_tile.rect.h = node2.child("image").attribute("height").as_int();
			} else if( node_name == "image" ) {
				// This is a tileset with just one image
				img_path = node2.attribute("source").value();
				cur_tile.rect.w = node2.attribute("width").as_int();
				cur_tile.rect.h = node2.attribute("height").as_int();
			} else {
				//No image to load
				
				//Is there a collision shape for this tile?
				if ( node2.child("objectgroup") ){
					parse_collison_obj(node2.child("objectgroup"), first_id + node2.attribute("id").as_int());
				}
				//We must have already loaded the image for this tile...
				continue;
			}
            
			SDL_Texture * texture;

			//Have we already loaded this texture?
			if (images.count(img_path) > 0){
				texture = images[img_path];
			} else {
				texture = loadTexture(img_path, renderer);
				images[img_path] = texture;
			}

			//Split a image into small tiles if it truly is a tileset
			if(cur_tile.rect.w > tile_w || cur_tile.rect.h > tile_h){
				for (int y = 0; (cur_tile.rect.h / (tile_h * (1+y))) >= 1; y++){
					for (int x = 0; (cur_tile.rect.w / (tile_w * (1+x))) >= 1; x++){
                        Tile sub_tile;
						sub_tile.texture = texture;
						sub_tile.rect.w = tile_w;
						sub_tile.rect.h = tile_h;
						sub_tile.rect.x = x*tile_w; 
						sub_tile.rect.y = y*tile_h;
						tile_tex.push_back(sub_tile);
					}
				}
			//The tile is the whole image
			} else {
				cur_tile.texture = texture;
				tile_tex.push_back(cur_tile);
				//Is there a collision shape for this tile?
				if ( node2.child("objectgroup") ){
					parse_collison_obj(node2.child("objectgroup"), first_id + node2.attribute("id").as_int());
				}
			}
		}
	}
	//Load all objects for this zone
	for (pugi::xml_node node = map.child("objectgroup"); node; node = node.next_sibling("objectgroup")) {
		string node_name( node.attribute("name").value() );
		if( node_name == "collision" ){
			parse_collison_obj(node, 0);
		} else {
			for(pugi::xml_node obj = node.first_child(); obj; obj = obj.next_sibling()){
				obj_list.push_back(new GameObject("box", tile_tex.at(obj.attribute("gid").as_int() - 1).texture, 10,
							obj.attribute("x").as_float(), obj.attribute("y").as_float())); 
			}
		}
	}
}

vector<SDL_Texture *> LevelZone::get_layers(SDL_Renderer *renderer){
	if( level_zone_layers.empty() ){
		for (unsigned int i = 0; i < level_tiles.size(); i++){
			SDL_Texture *texture;
			texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, zone_tile_w * zone_w, zone_tile_h * zone_h);
			SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
			SDL_SetRenderTarget(renderer, texture);
			SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
			SDL_RenderClear(renderer);
            
			for (unsigned int y = 0; y < level_tiles[i].size(); y++){
				for (unsigned int x = 0; x < level_tiles[i][y].size(); x++){
                    unsigned int index = level_tiles[i][y][x];
					if (index == 0) {
						//Empty tile
						continue;
					}
					SDL_Rect dest =  tile_tex[index -1].rect;
					dest.x = x * zone_tile_w;
					//Because we want the origin of the tile to be at the bottom left of the tile, we have to add an offset here
					dest.y = (y + 1) * zone_tile_h - tile_tex[index -1].rect.h;
					SDL_RenderCopy(renderer, tile_tex[index -1].texture, &tile_tex[index -1].rect, &dest);
				}
			}
			level_zone_layers.push_back(texture);
			// Change the target back to the default and then render the aux
			SDL_SetRenderTarget(renderer, NULL); //NULL SETS TO DEFAULT
		}
	}
	return level_zone_layers;
}

vector<vector<SDL_Point>> LevelZone::get_coll_vec(){
	vector<vector<SDL_Point>> shape_vec;
	for (unsigned int i = 0; i < level_tiles.size(); i++){
		for (unsigned int y = 0; y < level_tiles[i].size(); y++){
			for (unsigned int x = 0; x < level_tiles[i][y].size(); x++){
				unsigned int index = level_tiles[i][y][x];
				if (index == 0) {
					//Empty tile
					continue;
				}
				SDL_Rect dest =  tile_tex[index -1].rect;
				dest.x = x * zone_tile_w;
				//Because we want the origin of the tile to be at the bottom left of the tile, we have to add an offset here
				dest.y = (y + 1) * zone_tile_h - tile_tex[index -1].rect.h;
				for(unsigned int p = 0; p < tile_tex[index -1].coll.size(); p++){
					vector<SDL_Point> point_vec;
					for(unsigned int j = 0; j < tile_tex[index -1].coll[p].size(); j++){
						SDL_Point point;
						point.x = tile_tex[index -1].coll[p][j].x + dest.x;
						point.y = tile_tex[index -1].coll[p][j].y + dest.y;
						point_vec.push_back(point);
					}
					shape_vec.push_back(point_vec);
				}
			}
		}
	}
	//Draw zone collision layer
	for(unsigned int p = 0; p < zone_coll.size(); p++){
		vector<SDL_Point> point_vec;
		for(unsigned int j = 0; j < zone_coll[p].size(); j++){
			SDL_Point point;
			point.x = zone_coll[p][j].x;
			point.y = zone_coll[p][j].y;

			point_vec.push_back(point);
		}
		shape_vec.push_back(point_vec);
	}
	return shape_vec;
}

SDL_Texture *LevelZone::get_coll_layer(SDL_Renderer *renderer){
	SDL_Texture *texture;
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, zone_tile_w * zone_w, zone_tile_h * zone_h);
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(renderer, texture);
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	
	//Get all collsion shapes
	vector<vector<SDL_Point>> all_coll = get_coll_vec();
	
	//Draw zone collision layer
	for(unsigned int p = 0; p < all_coll.size(); p++){
		for(unsigned int j = 0; j < all_coll[p].size() - 1; j++){
			SDL_RenderDrawLine(renderer, all_coll[p][j].x, all_coll[p][j].y,
										 all_coll[p][j+1].x, all_coll[p][j+1].y);
		}
	}
	level_zone_layers.push_back(texture);
	// Change the target back to the default and then render the aux
	SDL_SetRenderTarget(renderer, NULL); //NULL SETS TO DEFAULT
	return texture;
}

void LevelZone::render_layers(SDL_Renderer *renderer, int off_x, int off_y){
	if( level_zone_layers.empty() ){
		get_layers(renderer);
		get_coll_layer(renderer);
	} 
	SDL_Rect dest;
	dest.x = off_x;
	dest.y = off_y;
	dest.w = zone_tile_w * zone_w;
	dest.h = zone_tile_h * zone_h;
	for (auto it = level_zone_layers.begin(); it != level_zone_layers.end(); ++it){
		SDL_RenderCopy(renderer, *it, NULL, &dest);
	}
}

//TODO only clean up images that are not needed for the next level if needed
void LevelZone::del_images(){
	 for (auto it = images.begin(); it != images.end(); ++it){
          SDL_DestroyTexture( it->second );
		  images.erase(it);
	 }
}

void LevelZone::del_layers(){
	//Clean up all layer textures
	 for (auto it = level_zone_layers.begin(); it != level_zone_layers.end(); ++it){
          SDL_DestroyTexture( *it );
	 }
     level_zone_layers.clear();
}

list<GameObject*> *LevelZone::get_objs(){
	return &obj_list;	
}

SDL_Point LevelZone::get_zone_sizes(){
	SDL_Point sizes = {zone_w, zone_h};
	return sizes;
}

LevelZone::~LevelZone(){
	del_layers();
}
