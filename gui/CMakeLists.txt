add_library(gui text_box.cpp)

include_directories(${CMAKE_SOURCE_DIR})

target_link_libraries(gui psf)
