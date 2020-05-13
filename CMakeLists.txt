cmake_minimum_required(VERSION 3.4)

project(SquiceLang)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 
set(CMAKE_VERBOSE_MAKEFILE TRUE) 

FILE(GLOB SRCFILES src/main.c)
FILE(GLOB INCFILES include/*.h)

set(SOURCE_FILES ${SRCFILES}
                 ${INCFILES})
				 
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_compile_definitions(SL_IMPLEMENTATION)

add_executable(SquiceLang ${SOURCE_FILES})

project(SquiceLangVM)