cmake_minimum_required(VERSION 3.8.0)
project("AtmosphereFramework")

file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/output/figures/animation"
                    "${CMAKE_SOURCE_DIR}/output/figures/final"
					"${CMAKE_SOURCE_DIR}/output/figures/animation"
					"${CMAKE_SOURCE_DIR}/output/egsr/1_renders"
					"${CMAKE_SOURCE_DIR}/output/egsr/2_abs_luminance"
					"${CMAKE_SOURCE_DIR}/output/egsr/3_chroma"
					"${CMAKE_SOURCE_DIR}/output/egsr/4_rel_error"
					"${CMAKE_SOURCE_DIR}/python_nn_training/res")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} 
                    "include"
                    "src")

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c" "src/*.h")

configure_file(src/raytracer/RootDir.h.in src/raytracer/RootDir.h)
include_directories(${CMAKE_BINARY_DIR}/src/raytracer)

# Add executable/project
add_executable(${PROJECT_NAME} ${SOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

target_link_libraries(${PROJECT_NAME})

# keras2cpp
add_subdirectory("${CMAKE_SOURCE_DIR}/3rdparty/keras2cpp")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/3rdparty/keras2cpp")
target_link_libraries(${PROJECT_NAME} "keras2cpp")

# Macro to preserve source files hierarchy in the IDE
macro(GroupSources curdir)
    file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${curdir} ${PROJECT_SOURCE_DIR}/${curdir}/*)

    foreach(child ${children})
        if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
            GroupSources(${curdir}/${child})
        else()
            string(REPLACE "/" "\\" groupname ${curdir})
            string(REPLACE "src" "Sources" groupname ${groupname})
            source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/${curdir}/${child})
        endif()
    endforeach()
endmacro()

# Run macro
GroupSources(src)

# Adds custom command to create needed directories before build
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/output/figures")