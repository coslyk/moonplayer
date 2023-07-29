
# Include MPV
include_directories(${MPV_INCLUDE_DIRS})

#include danmaku2ass
include_directories(${PROJECT_SOURCE_DIR}/3rdparty/danmaku2ass_cpp/include)

# Add source files
set(SOURCE_FILES
    accessManager.cpp
    application.cpp
    clipboard.cpp
    danmakuLoader.cpp
    dialogs.cpp
    downloader.cpp
    downloaderAbstractItem.cpp
    downloaderHlsItem.cpp
    downloaderItem.cpp
    fileDownloader.cpp
    jsapiObject.cpp
    main.cpp
    mpvObject.cpp
    parserBase.cpp
    parserLux.cpp
    parserYtdlp.cpp
    playlistModel.cpp
    plugin.cpp
    utils.cpp
    websiteSettings.cpp
)

# Add platform specific source files
if (APPLE)
    list(APPEND SOURCE_FILES
        platform/graphics_mac.cpp
        platform/paths_mac.cpp
    )
elseif (UNIX AND NOT APPLE)
    list(APPEND SOURCE_FILES
        platform/graphics_linux.cpp
        platform/paths_linux.cpp
    )
elseif (WIN32)
    list(APPEND SOURCE_FILES
        platform/graphics_win.cpp
        platform/paths_win.cpp
    )
endif()

# QML files
set(QML_FILES
    qml/main.qml
    qml/ConsoleDialog.qml
    qml/ControlBar.qml
    qml/CoverArea.qml
    qml/Downloader.qml
    qml/Explorer.qml
    qml/ImageButton.qml
    qml/MessageDialog.qml
    qml/OpenUrlDialog.qml
    qml/Playlist.qml
    qml/SelectionDialog.qml
    qml/Settings.qml
    qml/ShortcutsDialog.qml
    qml/Sidebar.qml
    qml/SubtitlesAndDanmaku.qml
    qml/TextInputDialog.qml
    qml/VideoOptions.qml
    qml_singletons/SkinColor.qml
)

# Create translations
set(TS_FILES
    l10n/zh_CN.ts
)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/l10n)
qt6_create_translation(QM_FILES ${SOURCE_FILES} ${QML_FILES} l10n/zh_CN.ts)

# Add MacOS bundle files
add_project_meta(META_FILES_TO_INCLUDE)
if (APPLE)
    # Fix translation on macOS 10.14 Mojave and above
    add_custom_target(build-time-make-directory ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_NAME}.app/Contents/Resources/zh_CN.lproj")
endif()


# Create excutable
qt_add_executable(${PROJECT_NAME}
    ${SOURCE_FILES}
    ${QM_FILES}
    resources_qt6.qrc
    l10n/l10n.qrc
    ${META_FILES_TO_INCLUDE})

# Enable Automoc and Autorcc for Qt
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    MACOSX_BUNDLE ON
    WIN32_EXECUTABLE TRUE
)

# Add qml files and resources
set_source_files_properties(qml_singletons/SkinColor.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(${PROJECT_NAME}
    URI com.github.coslyk.moonplayer
    VERSION 1.0
    QML_FILES
        ${QML_FILES}
    RESOURCES
        images/net_grey.png
        images/net_grey_on.png
        images/net_lightgrey.png
        images/net_lightgrey_on.png
        images/pause_grey.png
        images/pause_grey_on.png
        images/pause_lightgrey.png
        images/pause_lightgrey_on.png
        images/play_grey.png
        images/play_grey_on.png
        images/play_lightgrey.png
        images/play_lightgrey_on.png
        images/playlist_grey.png
        images/playlist_grey_on.png
        images/playlist_lightgrey.png
        images/playlist_lightgrey_on.png
        images/settings_grey.png
        images/settings_grey_on.png
        images/settings_lightgrey.png
        images/settings_lightgrey_on.png
        images/stop_grey.png
        images/stop_grey_on.png
        images/stop_lightgrey.png
        images/stop_lightgrey_on.png
        images/volume_grey.png
        images/volume_grey_on.png
        images/volume_lightgrey.png
        images/volume_lightgrey_on.png
)

# Link
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Qml
    Qt6::Quick
    Qt6::Gui
    Qt6::Network
    Qt6::OpenGL
    ${MPV_LIBRARY}
    Danmaku2AssCPP)

fix_win_compiler()

# Enable all compilation warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

# Install on Linux
if (UNIX AND NOT APPLE)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
    install(DIRECTORY ../res/icons/ DESTINATION share/icons)
    install(FILES ../res/com.github.coslyk.MoonPlayer.appdata.xml DESTINATION share/metainfo)
    install(FILES ../res/com.github.coslyk.MoonPlayer.desktop DESTINATION share/applications)
endif()
