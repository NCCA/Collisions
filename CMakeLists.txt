cmake_minimum_required(VERSION 3.12)
# Name of the project
project(TextureDemosBuildAll)

add_subdirectory(${PROJECT_SOURCE_DIR}/BoundingBox/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/RaySphere/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/RayTriangle/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/SpherePlane/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/SphereSphere/ )
