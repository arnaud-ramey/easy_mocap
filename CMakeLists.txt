project(easy_mocap)
cmake_minimum_required(VERSION 2.8)
# Debug, Release, RelWithDebInfo and MinSizeRe
set(CMAKE_BUILD_TYPE RelWithDebInfo)
SET(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra") # add extra warnings

find_package(GLUT REQUIRED)
find_package(JPEG REQUIRED)
find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/easy_mocap_path.h.in"
               "${PROJECT_BINARY_DIR}/easy_mocap_path.h")

add_library(easy_mocap
  libs/bitmap/Bitmap.cpp
  libs/rtg/RTGlib.c
  DisjointSets.cpp
  ImageUtils.cpp
  HsvFilter.cpp
  ImageAnalyzer.cpp
  VirtualBody.cpp
  OrientationFinder.cpp)
add_executable(test_easy_mocap test.cpp)
target_link_libraries(test_easy_mocap easy_mocap
  ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${OPENGL_LIBRARIES}
  ${JPEG_LIBRARIES} ${GLUT_LIBRARIES})
