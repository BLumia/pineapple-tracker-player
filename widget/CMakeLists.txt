cmake_minimum_required(VERSION 3.8)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(ECM 5.83.0 NO_MODULE)

if(ECM_FOUND)
    set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
    include(ECMAddAppIcon)
endif()

set(PROJECT_SOURCES
    main.cpp
    mainwindow.h mainwindow.cpp mainwindow.ui
    instrumentsmodel.h instrumentsmodel.cpp
)

if (WIN32)
    list(APPEND PROJECT_SOURCES "${PROJECT_SOURCE_DIR}/assets/pineapple-tracker-player.rc")
endif()

qt_add_executable(pineapple-tracker-player
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

qt_add_translations(pineapple-tracker-player
TS_FILES
    ../quick/translations/pineapple-tracker-player.ts
    ../quick/translations/pineapple-tracker-player_nl.ts
    ../quick/translations/pineapple-tracker-player_pl.ts
    ../quick/translations/pineapple-tracker-player_tr.ts
    ../quick/translations/pineapple-tracker-player_zh_CN.ts
)

target_compile_definitions(pineapple-tracker-player PRIVATE
    PTPLAY_VERSION_STRING="${CMAKE_PROJECT_VERSION}"
)

if(ECM_FOUND)
    ecm_add_app_icon(pineapple-tracker-player
    ICONS
        ../dist/512-pineapple-tracker-player.png
        ../dist/256-pineapple-tracker-player.png
        ../dist/64-pineapple-tracker-player.png
        ../dist/pineapple-tracker-player.svg
    )
endif()

qt_add_resources(pineapple-tracker-player resources.qrc
    PREFIX "/icons"
    FILES ../dist/pineapple-tracker-player.svg
)

target_link_libraries(pineapple-tracker-player PRIVATE player-shared Qt::Widgets)

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
