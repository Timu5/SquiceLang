cmake_minimum_required(VERSION 3.4)

project(SquiceLangSDL2)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 
set(CMAKE_VERBOSE_MAKEFILE TRUE) 

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

FILE(GLOB SRCFILES mainsdl.c)
FILE(GLOB INCFILES ../../include/*.h)

set(SOURCE_FILES ${SRCFILES}
                 ${INCFILES})
				 
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

#add_compile_definitions(SL_IMPLEMENTATION)

add_executable(SquiceLangSDL2 ${SOURCE_FILES})
target_link_libraries(SquiceLangSDL2 ${SDL2_LIBRARIES})
