cmake_minimum_required(VERSION 3.4)

project(SquiceLang)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 
set(CMAKE_VERBOSE_MAKEFILE TRUE) 

FILE(GLOB SRCFILES tools/interpreter.c)
FILE(GLOB INCFILES include/*.h)

set(SOURCE_FILES ${SRCFILES}
                 ${INCFILES})
				 
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_compile_definitions(SL_IMPLEMENTATION)

add_executable(SquiceLang ${SOURCE_FILES})

project(SquiceLangVM)