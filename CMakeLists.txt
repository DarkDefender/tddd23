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

add_executable(${PROJECT_NAME} main.cpp timer.cpp)

include(FindPkgConfig)

pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2TTF REQUIRED SDL2_ttf>=2.0.12)
pkg_search_module(BULLET REQUIRED bullet)

include_directories(${CMAKE_SOURCE_DIR} ${SDL2_INCLUDE_DIR}
	${SDL2TTF_INCLUDE_DIRS} ${BULLET_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} gui ${SDL2_LIBRARIES}
	${SDL2TTF_LIBRARIES} ${BULLET_LIBRARIES})
