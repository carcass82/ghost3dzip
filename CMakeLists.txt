cmake_minimum_required(VERSION 2.8.4)

project(SPECCOMP)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${SPECCOMP_SOURCE_DIR}/CMakeModules/")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_FLAGS, "-fpermissive")

add_subdirectory(${SPECCOMP_SOURCE_DIR}/3dzip)
include_directories(${HDS_SOURCE_DIR})
include_directories(${3DZIP_SOURCE_DIR})
include_directories(${SPECCOMPO_SOURCE_DIR}/3dzip/libhds)

add_subdirectory(${SPECCOMP_SOURCE_DIR}/src)
include_directories(${SPECCOMP_SOURCE_DIR})
