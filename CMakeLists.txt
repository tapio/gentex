project(GenTex)
cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

# Avoid source tree pollution
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "In-source builds are not permitted. Make a separate folder for building:\nmkdir build; cd build; cmake ..\nBefore that, remove the files already created:\nrm -rf CMakeCache.txt CMakeFiles")
endif()

# Add a sensible build type default and warning because empty means no optimization and no debug info.
if(NOT CMAKE_BUILD_TYPE)
	message("WARNING: CMAKE_BUILD_TYPE is not defined!\n         Defaulting to CMAKE_BUILD_TYPE=Release.\n         Use ccmake to set a proper value.")
	SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# Enable C++11 support and other compiler flags...
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
	message("Detected GCC version ${GCC_VERSION}")
	if (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
		set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra ${CMAKE_CXX_FLAGS}")
	elseif (GCC_VERSION VERSION_GREATER 4.6 OR GCC_VERSION VERSION_EQUAL 4.6)
		set(CMAKE_CXX_FLAGS "-std=c++0x -Wall -Wextra ${CMAKE_CXX_FLAGS}")
	else()
		message(FATAL_ERROR "Your GCC compiler does not support c++11, please install at least gcc 4.6")
	endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra ${CMAKE_CXX_FLAGS}")
endif()

# Libs
include_directories(lib)
include_directories(include)

# Files
file(GLOB SOURCE_FILES "src/*.cpp")
file(GLOB HEADER_FILES "include/*.hpp")
file(GLOB_RECURSE LIB_FILES "lib/*.cpp")
set(SOURCES ${SOURCE_FILES} ${HEADER_FILES} ${LIB_FILES})

add_executable(basic "tests/basic.cpp" ${SOURCES})
add_executable(gentex "tests/tool.cpp" ${SOURCES})
