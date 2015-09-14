cmake_minimum_required(VERSION 2.8.11)
# Name of the project
set(PROJECT_NAME TextureDemos)
project(${PROJECT_NAME})

add_subdirectory(${PROJECT_SOURCE_DIR}/BoundingBox/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/RaySphere/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/RayTriangle/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/SpherePlane/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/SphereSphere/ )
