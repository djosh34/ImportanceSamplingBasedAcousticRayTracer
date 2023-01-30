cmake_minimum_required(VERSION 3.20)
project(Raytracer)

include(ExternalProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-std=c++17 -pthread")

# boost
find_package(Boost 1.40 COMPONENTS filesystem REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )


# assimp
include_directories(${CMAKE_SOURCE_DIR}/external/assimp/include)
link_directories(${CMAKE_SOURCE_DIR}/external/assimp/lib)

# json
find_package(nlohmann_json 3.2.0 REQUIRED)



# include executables
add_library(${PROJECT_NAME}LIBS STATIC
        include/window.cpp
        include/trackball.cpp
        )

# executables
add_executable(${PROJECT_NAME}
        src/main.cpp
        src/Mesh.cpp
        src/draw.cpp
        src/rays/Ray.cpp
        src/rays/RayTracing.cpp
        src/rays/directionGenerator.cpp
        src/rays/Energy.cpp
        src/Receiver.cpp
        src/config.cpp
        src/auto_runner.cpp
        src/rays/Gmm.cpp

        src/rays/projections.cpp

        include/helpers/printHelper.cpp

        )

# directories

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)



# glfw
find_package(glfw3 3.3 REQUIRED)
target_link_libraries(Raytracer glfw)

# opengl
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )



target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} )
target_link_libraries(${PROJECT_NAME} assimp)
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}LIBS)
