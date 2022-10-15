cmake_minimum_required(VERSION 3.8)

project(pineapple-tracker-player VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(PkgConfig REQUIRED)

pkg_check_modules(OpenMPT REQUIRED libopenmpt IMPORTED_TARGET)
pkg_check_modules(PortAudio REQUIRED portaudiocpp IMPORTED_TARGET)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.h mainwindow.cpp mainwindow.ui
        player.h player.cpp
)

qt_add_executable(pineapple-tracker-player
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(pineapple-tracker-player PRIVATE Qt::Widgets PkgConfig::OpenMPT PkgConfig::PortAudio)

set_target_properties(pineapple-tracker-player PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS pineapple-tracker-player
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(pineapple-tracker-player)
endif()
