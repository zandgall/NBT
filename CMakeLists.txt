cmake_minimum_required(VERSION 3.22.1)
project(nbt-example VERSION 0.2 DESCRIPTION "NBT Example")

# Grab all the files in /src/...
file(GLOB_RECURSE SOURCE_FILES example.c nbt.c)
file(GLOB_RECURSE HEADER_FILES nbt.h)

# Mark down what libraries are to be included, along with glad's header file directory
set(LIBS z m)

# Link the executable with its files, including glad and included libraries
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${LIBS})

configure_file(./out ./out COPYONLY)

# Confirm debug and release compile options
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:Debug>:-DDEBUG;-g;-Wall>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:Release>:-O3>")
