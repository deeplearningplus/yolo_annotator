cmake_minimum_required(VERSION 3.10)
project(yolo_annotator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(yolo_annotator main.cpp)
target_link_libraries(yolo_annotator ${OpenCV_LIBS})
