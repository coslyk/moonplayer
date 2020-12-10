
# Include Qt5Gui_Private
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

# Include MPV
include_directories(${MPV_INCLUDE_DIRS})

# Include Qt5_X11Extras
if (UNIX AND NOT APPLE)
    include_directories(${Qt5X11Extras_INCLUDE_DIRS})
endif()

#include danmaku2ass
include_directories(${PROJECT_SOURCE_DIR}/3rdparty/danmaku2ass_cpp/include)

# Add source files
set(SOURCE_FILES
    accessManager.cpp
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
    parserGtv.cpp
    parserYkdl.cpp
    parserYoutubedl.cpp
    playlistModel.cpp
    plugin.cpp
    utils.cpp
)

# Add platform specific source files
if (APPLE)
    list(APPEND SOURCE_FILES
        platform/application_mac.cpp
        platform/graphics_mac.cpp
        platform/paths_mac.cpp
    )
elseif (UNIX AND NOT APPLE)
    list(APPEND SOURCE_FILES
        platform/application_no_mac.cpp
        platform/graphics_linux.cpp
        platform/paths_linux.cpp
    )
elseif (WIN32)
    list(APPEND SOURCE_FILES
        platform/application_no_mac.cpp
        platform/graphics_win.cpp
        platform/paths_win.cpp
    )
endif()


# Create translations
qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} l10n/zh_CN.ts)
configure_file(l10n/l10n.qrc ${CMAKE_BINARY_DIR}/src COPYONLY)

# Compile QML files
if (COMPILE_QML)
    qtquick_compiler_add_resources(QML_FILES qmls.qrc)
else()
    qt5_add_resources(QML_FILES qmls.qrc)
endif()


# Add MacOS bundle files
add_project_meta(META_FILES_TO_INCLUDE)
if (APPLE)
    # Fix translation on macOS 10.14 Mojave and above
    add_custom_target(build-time-make-directory ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_NAME}.app/Contents/Resources/zh_CN.lproj")
endif()


# Create excutable
add_executable(${PROJECT_NAME} ${OS_BUNDLE} # Expands to WIN32 or MACOS_BUNDLE depending on OS
    ${SOURCE_FILES}
    ${QM_FILES}
    ${QML_FILES}
    resources.qrc
    ${CMAKE_BINARY_DIR}/src/l10n.qrc
    ${META_FILES_TO_INCLUDE})

fix_win_compiler()

# Link libraries
target_link_libraries(${PROJECT_NAME}
    Qt5::Qml
    Qt5::Quick
    Qt5::Gui
    Qt5::Network
    ${Qt5X11Extras_LIBRARIES}
    ${MPV_LIBRARY}
    Danmaku2AssCPP)

# Enable Automoc and Autorcc for Qt
set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC ON AUTORCC ON)

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
