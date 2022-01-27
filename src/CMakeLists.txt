
# Include Qt Gui private headers
if (USE_QT6)
    include_directories(${Qt6Gui_PRIVATE_INCLUDE_DIRS})
else()
    include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})
endif()

# Include MPV
include_directories(${MPV_INCLUDE_DIRS})

#include danmaku2ass
include_directories(${PROJECT_SOURCE_DIR}/3rdparty/danmaku2ass_cpp/include)

# Add source files
set(SOURCE_FILES
    accessManager.cpp
    application.cpp
    danmakuLoader.cpp
    dialogs.cpp
    downloader.cpp
    downloaderAbstractItem.cpp
    downloaderHlsItem.cpp
    downloaderItem.cpp
    fileDownloader.cpp
    fileOpenDialog.cpp
    fontDialog.cpp
    jsapiObject.cpp
    main.cpp
    mpvObject.cpp
    parserBase.cpp
    parserYkdl.cpp
    parserYtdlp.cpp
    playlistModel.cpp
    plugin.cpp
    utils.cpp
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


# Create translations
if (USE_QT6)
    qt6_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} l10n/zh_CN.ts)
    configure_file(l10n/l10n.qrc ${CMAKE_BINARY_DIR}/src COPYONLY)
else()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} l10n/zh_CN.ts)
    configure_file(l10n/l10n.qrc ${CMAKE_BINARY_DIR}/src COPYONLY)

    # Compile QML files
    if (COMPILE_QML)
        qtquick_compiler_add_resources(QML_FILES qmls.qrc)
    else()
        qt5_add_resources(QML_FILES qmls.qrc)
    endif()
endif()

# Add MacOS bundle files
add_project_meta(META_FILES_TO_INCLUDE)
if (APPLE)
    # Fix translation on macOS 10.14 Mojave and above
    add_custom_target(build-time-make-directory ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_NAME}.app/Contents/Resources/zh_CN.lproj")
endif()

# Link libraries
if (USE_QT6)
    # Create excutable
    add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
        ${SOURCE_FILES}
        ${QM_FILES}
        resources_qt6.qrc
        ${CMAKE_BINARY_DIR}/src/l10n.qrc
        ${META_FILES_TO_INCLUDE})

    # Enable Automoc and Autorcc for Qt
    set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC ON AUTORCC ON)

    # Add qml files and resources
    qt6_add_qml_module(${PROJECT_NAME}
        URI moonplayer_qml
        VERSION 1.0
        NO_GENERATE_QMLDIR
        NO_GENERATE_QMLTYPES
        QML_FILES
            qml/main.qml
            qml/ConsoleDialog.qml
            qml/ControlBar.qml
            qml/DanmakuOptionsDialog.qml
            qml/Downloader.qml
            qml/Explorer.qml
            qml/FileOpenDialog.qml
            qml/FontDialog.qml
            qml/MessageDialog.qml
            qml/OpenUrlDialog.qml
            qml/Playlist.qml
            qml/SelectionDialog.qml
            qml/Settings.qml
            qml/SkinColor.qml
            qml/TextInputDialog.qml
            qml/VideoOptionsDialog.qml
            qml/classicUI/CustomWidgets/CustomDialog.qml
            qml/classicUI/CustomWidgets/CustomImageButton.qml
            qml/classicUI/CustomWidgets/CustomTabDialog.qml
            qml/classicUI/CustomWidgets/CustomWindow.qml
            qml/modernUI/CustomWidgets/CustomDialog.qml
            qml/modernUI/CustomWidgets/CustomImageButton.qml
            qml/modernUI/CustomWidgets/CustomTabDialog.qml
            qml/modernUI/CustomWidgets/CustomWindow.qml
        RESOURCES
            qml/classicUI/CustomWidgets/qmldir
            qml/modernUI/CustomWidgets/qmldir
            images/net_grey.png
            images/net_lightgrey.png
            images/pause_grey.png
            images/pause_lightgrey.png
            images/play_grey.png
            images/play_lightgrey.png
            images/playlist_grey.png
            images/playlist_lightgrey.png
            images/settings_grey.png
            images/settings_lightgrey.png
            images/stop_grey.png
            images/stop_lightgrey.png
            images/volume_grey.png
            images/volume_lightgrey.png
    )

    # Link
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Qml
        Qt6::Quick
        Qt6::Gui
        Qt6::Network
        Qt6::OpenGL
        ${MPV_LIBRARY}
        Danmaku2AssCPP)
else()
    # Create excutable
    add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
        ${SOURCE_FILES}
        ${QM_FILES}
        ${QML_FILES}
        resources.qrc
        ${CMAKE_BINARY_DIR}/src/l10n.qrc
        ${META_FILES_TO_INCLUDE})

    # Enable Automoc and Autorcc for Qt
    set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC ON AUTORCC ON)

    # Link
    target_link_libraries(${PROJECT_NAME}
        Qt5::Qml
        Qt5::Quick
        Qt5::Gui
        Qt5::Network
        ${MPV_LIBRARY}
        Danmaku2AssCPP)
endif()

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
