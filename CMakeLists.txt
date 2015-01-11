cmake_minimum_required(VERSION 2.8)
project(easy_mocap)
find_package(GLUT REQUIRED)
find_package(JPEG REQUIRED)
find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

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
