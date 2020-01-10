
# Include Qt5Gui_Private
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

# Include MPV
include_directories(${MPV_INCLUDE_DIRS})

# Include Qt5_X11Extras
if (UNIX AND NOT APPLE)
    include_directories(${Qt5X11Extras_INCLUDE_DIRS})
endif()

# Add source files
set(SOURCE_FILES
    accessManager.cpp
    console.cpp
    danmakuLoader.cpp
    downloader.cpp
    downloaderAbstractItem.cpp
    downloaderHlsItem.cpp
    downloaderMultipleItem.cpp
    downloaderSingleItem.cpp
    jsapiObject.cpp
    main.cpp
    mpvObject.cpp
    parserBase.cpp
    parserYkdl.cpp
    parserYoutubedl.cpp
    playlistModel.cpp
    plugin.cpp
    utils.cpp
)

# Add platform specific source files
if (UNIX AND NOT APPLE)
    list(APPEND SOURCE_FILES
        platform/application_no_mac.cpp
        platform/detectOpengl_linux.cpp
        platform/paths_linux.cpp
    )
endif()

if (APPLE)
    list(APPEND SOURCE_FILES
        platform/application_mac.cpp
        platform/detectOpengl_mac.cpp
        platform/paths_mac.cpp
    )
endif()

if (WIN32)
    list(APPEND SOURCE_FILES
        platform/application_no_mac.cpp
        platform/detectOpengl_win.cpp
        platform/paths_win.cpp
    )
endif()

# AUTOMOC seems not able to find this file
qt5_wrap_cpp(SOURCE_FILES platform/application.h TARGET ${PROJECT_NAME})


# Create translations
qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} l10n/zh_CN.ts)
configure_file(l10n/l10n.qrc ${CMAKE_BINARY_DIR}/src COPYONLY)

# Add MacOS bundle files
add_project_meta(META_FILES_TO_INCLUDE)
if (APPLE)
    set_source_files_properties(
        scripts/danmaku2ass.py
        scripts/update-parsers.sh
        PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources)
    list(APPEND META_FILES_TO_INCLUDE
        scripts/danmaku2ass.py
        scripts/update-parsers.sh
    )
    file(COPY /usr/local/opt/ffmpeg-lite/bin/ffmpeg DESTINATION "${PROJECT_NAME}.app/Contents/MacOS/")
endif()


# Create excutable
add_executable(${PROJECT_NAME} ${OS_BUNDLE} # Expands to WIN32 or MACOS_BUNDLE depending on OS
    ${SOURCE_FILES} ${META_FILES_TO_INCLUDE} ${CMAKE_BINARY_DIR}/src/l10n.qrc ${QM_FILES} resources.qrc)

fix_win_compiler()

target_link_libraries(${PROJECT_NAME} Qt5::Qml Qt5::Quick Qt5::Gui Qt5::Widgets Qt5::Network ${Qt5X11Extras_LIBRARIES} ${MPV_LIBRARY})


# Install on Linux
if (UNIX AND NOT APPLE)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
    install(DIRECTORY scripts/ DESTINATION share/moonplayer)
    install(DIRECTORY ../res/icons/ DESTINATION share/icons)
    install(FILES ../res/com.github.coslyk.MoonPlayer.appdata.xml DESTINATION share/metainfo)
    install(FILES ../res/com.github.coslyk.MoonPlayer.desktop DESTINATION share/applications)
endif()
