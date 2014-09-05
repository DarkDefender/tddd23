cmake_minimum_required (VERSION 2.6)
project (Tutorial)

find_package(ZLIB)

# Needed check for libpsf
if ( NOT ZLIB_FOUND )
	set(NO_ZLIB 1)
endif( NOT ZLIB_FOUND )

add_subdirectory(libs)

add_executable(${PROJECT_NAME} main.cpp)

include(FindPkgConfig)

pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2TTF REQUIRED SDL2_ttf>=2.0.12)
include_directories(${SDL2_INCLUDE_DIR} ${SDL2TTF_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} libpsf ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES})
