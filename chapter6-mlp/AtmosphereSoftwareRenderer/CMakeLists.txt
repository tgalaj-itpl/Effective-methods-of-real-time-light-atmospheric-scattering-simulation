# Copyright (C) 2020 Tomasz Gałaj

cmake_minimum_required(VERSION 3.9.2 FATAL_ERROR)
project(AtmosphereSoftwareRenderer VERSION 0.1)

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	 ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
	 ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
	
# Add header files
file(GLOB_RECURSE HEADER_FILES 
	 ${CMAKE_SOURCE_DIR}/include/*.h
	 ${CMAKE_SOURCE_DIR}/include/*.hpp
	 ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
	 ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)

# assimp
find_library(ASSIMP_LIBRARY "assimp" "/usr/lib" "/usr/local/lib")
find_path(ASSIMP_INCLUDE_DIR "assimp/mesh.h" "/usr/include" "/usr/local/include")

if((NOT ASSIMP_LIBRARY) OR (NOT ASSIMP_INCLUDE_DIR))
	set(ASSIMP_DIR "${CMAKE_SOURCE_DIR}/3rdparty/assimp")
	
	set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Build package with shared libraries.")
	set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "If the supplementary tools for Assimp are built in addition to the library.")
	set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "If the test suite for Assimp is built in addition to the library.")

    add_subdirectory("${ASSIMP_DIR}")

	set(ASSIMP_LIBRARY "assimp")
	set(ASSIMP_INCLUDE_DIR "${ASSIMP_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/3rdparty/assimp/include")
endif()

# keras2cpp
add_subdirectory("${CMAKE_SOURCE_DIR}/3rdparty/keras2cpp")

# Define the executable
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PUBLIC "${ASSIMP_INCLUDE_DIR}")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/3rdparty/keras2cpp") 

target_link_libraries(${PROJECT_NAME} "${ASSIMP_LIBRARY}")
target_link_libraries(${PROJECT_NAME} "keras2cpp")

if(CMAKE_SIZEOF_VOID_P EQUAL 8) #64bits
    # 64 bits
	target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/bin64/SDL2.lib ${CMAKE_SOURCE_DIR}/bin64/SDL2main.lib ${CMAKE_SOURCE_DIR}/bin64/SDL2test.lib)
	
	add_custom_command(TARGET  ${PROJECT_NAME} POST_BUILD
					   COMMAND ${CMAKE_COMMAND} -E copy
							   ${CMAKE_SOURCE_DIR}/bin64/SDL2.dll
							   ${CMAKE_CURRENT_BINARY_DIR})
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4) #32bits
    # 32 bits
	target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/bin32/SDL2.lib ${CMAKE_SOURCE_DIR}/bin32/SDL2main.lib ${CMAKE_SOURCE_DIR}/bin32/SDL2test.lib)
	
	add_custom_command(TARGET  ${PROJECT_NAME} POST_BUILD
					   COMMAND ${CMAKE_COMMAND} -E copy
							   ${CMAKE_SOURCE_DIR}/bin32/SDL2.dll
							   ${CMAKE_CURRENT_BINARY_DIR})
endif()

add_custom_command(TARGET  ${PROJECT_NAME} POST_BUILD
				   COMMAND ${CMAKE_COMMAND} -E copy_directory
						   ${CMAKE_SOURCE_DIR}/res
						   ${CMAKE_CURRENT_BINARY_DIR}/res)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "sources"   FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_SOURCE_DIR} PREFIX "headers"   FILES ${HEADER_FILES})