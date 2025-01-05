cmake_minimum_required(VERSION 3.22)

project(pineapple-tracker-player VERSION 0.3.0 LANGUAGES CXX)

# https://forum.qt.io/topic/155823/cmake-does-not-work-with-qt-6-7-0
# https://bugreports.qt.io/browse/QTBUG-126167
find_package(Qt6 REQUIRED COMPONENTS Core LinguistTools)

add_subdirectory(player)
add_subdirectory(widget)
add_subdirectory(quick)
