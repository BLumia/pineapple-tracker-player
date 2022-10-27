cmake_minimum_required(VERSION 3.8)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

set(PROJECT_SOURCES
    main.cpp
)

qt_add_executable(pineapple-tracker-player-quick
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

qt_add_qml_module(pineapple-tracker-player-quick
URI player
VERSION 1.0
QML_FILES
    main.qml
    InstrumentView.qml
    TrackerView.qml
    components/InstrumentItem.qml
)

qt_add_resources(pineapple-tracker-player-quick resources.qrc
    PREFIX "/icons"
    FILES ../dist/pineapple-tracker-player.svg
)

target_link_libraries(pineapple-tracker-player-quick PRIVATE player-shared Qt::Quick)

set_target_properties(pineapple-tracker-player-quick PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS pineapple-tracker-player-quick
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# install icon
install (
    FILES ../dist/pineapple-tracker-player.svg
    DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps"
    RENAME pineapple-tracker-player.svg
)

# install shortcut
install (
    FILES ../dist/net.blumia.pineapple-tracker-player.desktop
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications"
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(pineapple-tracker-player-quick)
endif()
