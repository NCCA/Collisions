cmake_minimum_required(VERSION 3.12)
#-------------------------------------------------------------------------------------------
# I'm going to use vcpk in most cases for our install of 3rd party libs
# this is going to check the environment variable for CMAKE_TOOLCHAIN_FILE and this must point to where
# vcpkg.cmake is in the University this is set in your .bash_profile to
# export CMAKE_TOOLCHAIN_FILE=/public/devel/2020/vcpkg/scripts/buildsystems/vcpkg.cmake
#-------------------------------------------------------------------------------------------
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{CMAKE_TOOLCHAIN_FILE})
   set(CMAKE_TOOLCHAIN_FILE $ENV{CMAKE_TOOLCHAIN_FILE})
endif()
# Name of the project
project(TextureDemosBuildAll)

add_subdirectory(${PROJECT_SOURCE_DIR}/BoundingBox/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/RaySphere/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/RayTriangle/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/SpherePlane/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/SphereSphere/ )
