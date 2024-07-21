cmake_minimum_required(VERSION 3.8)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui)
find_package(PkgConfig REQUIRED)

pkg_check_modules(OpenMPT REQUIRED libopenmpt IMPORTED_TARGET)
pkg_check_modules(PortAudio REQUIRED portaudiocpp IMPORTED_TARGET)

set(PROJECT_SOURCES
    player.h player.cpp
    util.h util.cpp
    playlistmanager.h playlistmanager.cpp
)

qt_add_library(player-shared OBJECT
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_include_directories(player-shared PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(player-shared PRIVATE Qt::Gui PkgConfig::OpenMPT PkgConfig::PortAudio)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(player-shared)
endif()
