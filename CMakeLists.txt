cmake_minimum_required (VERSION 2.6)
project (Tutorial)

find_package(ZLIB)

list( APPEND CMAKE_CXX_FLAGS "-Wall -ggdb -std=c++11 ${CMAKE_CXX_FLAGS}")

# Needed check for libpsf
if ( NOT ZLIB_FOUND )
	set(NO_ZLIB 1)
endif( NOT ZLIB_FOUND )

add_subdirectory(libs)
add_subdirectory(gui)

add_executable(${PROJECT_NAME} main.cpp timer.cpp level.cpp sdl_h_func.cpp
	obj.cpp)

include(FindPkgConfig)

# Add modules for packages not in vanilla cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2TTF REQUIRED SDL2_ttf>=2.0.12)
pkg_search_module(SDL2IMAGE REQUIRED SDL2_image)
pkg_search_module(BULLET REQUIRED bullet)
find_package(PugiXML REQUIRED)

include_directories(${CMAKE_SOURCE_DIR} ${SDL2_INCLUDE_DIR}
	${SDL2TTF_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS} ${PUGIXML_INCLUDE_DIRS} ${BULLET_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} gui ${SDL2_LIBRARIES}
	${SDL2TTF_LIBRARIES} ${SDL2IMAGE_LIBRARIES} ${PUGIXML_LIBRARIES} ${BULLET_LIBRARIES})
