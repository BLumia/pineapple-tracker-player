cmake_minimum_required(VERSION 3.19)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core Quick LinguistTools Qml)
find_package(ECM 5.83.0 NO_MODULE)

if(ECM_FOUND)
    set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
    include(ECMAddAppIcon)
endif()

set(PROJECT_SOURCES
    main.cpp
)

qt_add_executable(pineapple-tracker-player-quick
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

if(ECM_FOUND)
    ecm_add_app_icon(pineapple-tracker-player-quick
    ICONS
        ../dist/512-pineapple-tracker-player.png
        ../dist/256-pineapple-tracker-player.png
        ../dist/64-pineapple-tracker-player.png
        ../dist/pineapple-tracker-player.svg
    )
endif()

qt_add_qml_module(pineapple-tracker-player-quick
URI player
VERSION 1.0
QML_FILES
    main.qml
    InstrumentView.qml
    TrackerView.qml
    components/InstrumentItem.qml
)

qt_add_translations(pineapple-tracker-player-quick
TS_FILES
    translations/pineapple-tracker-player.ts
    translations/pineapple-tracker-player_nl.ts
    translations/pineapple-tracker-player_pl.ts
    translations/pineapple-tracker-player_tr.ts
    translations/pineapple-tracker-player_zh_CN.ts
)

qt_add_resources(pineapple-tracker-player-quick resources.qrc
    PREFIX "/icons"
    FILES ../dist/pineapple-tracker-player.svg
)

target_link_libraries(pineapple-tracker-player-quick PRIVATE player-shared Qt::Quick Qt::Qml)

set_target_properties(pineapple-tracker-player-quick PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# qt_generate_deploy_qml_app_script(
#     TARGET pineapple-tracker-player-quick
#     OUTPUT_SCRIPT quick_deploy_script
#     NO_UNSUPPORTED_PLATFORM_ERROR
# )
# install(SCRIPT ${quick_deploy_script})

install(TARGETS pineapple-tracker-player-quick
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# install icon
install (
    FILES ../dist/pineapple-tracker-player.svg
    DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps"
    RENAME pineapple-tracker-player.svg
)

# install shortcut
install (
    FILES ../dist/net.blumia.pineapple-tracker-player.desktop
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications"
)

if(QT_VERSION_MAJOR EQUAL 6)
#    qt_import_qml_plugins(pineapple-tracker-player-quick) # do we really need this?
    qt_finalize_executable(pineapple-tracker-player-quick)
endif()
