### version 3.9 (2021.11.23)

1. Replace youtube-dl with yt-dlp
1. Qt updates to 6.2 on Windows and Linux AppImage
1. Add switch for autoplay after adding files (#115)
1. Remove update check (#112) 

### version 3.8 (2021.05.01)

1. Fix segmentation fault when opening some YouTube videos (#106)
1. Improvements in Classic UI
1. Classic UI has a native look on Windows now
1. Fix cannot select untitled audio tracks

### version 3.7 (2021.01.16)

1. New logo
1. Add dark theme support for classic UI (requires Qt >= 5.15)
1. Add cache size settings
1. Fix video cannot be downloaded in Flatpak version

### version 3.6 (2020.11.25)

1. Youtube-dl is back
1. Fix sometimes pause/play doesn't work
1. Remove dependency on QtWidgets
1. Performance optimization

### version 3.5.2 (2020.10.25)

1. Remove built-in youtube-dl due to DCMA takedown ([#100](https://github.com/coslyk/moonplayer/issues/100))

### version 3.5 (2020.10.15)

1. Make titlebar larger ([#98](https://github.com/coslyk/moonplayer/issues/98))
2. Modern UI now fully supports Wayland (Requires Qt >= 5.15)
3. Classic UI now supports macOS
4. Support nvdec hardware decoding
5. Fix AppImage cannot call ffmpeg

### version 3.4 (2020.09.05)

1. Performance improvements
2. Warn users if the proxy format is incorrect
3. Add options to hide specific danmaku comments
4. Reduce App size

### version 3.3 (2020.07.05)

1. Add new plugin APIs to show dialogs and store configurations
2. Support multilingual plugin name and description
3. Fix wrong danmaku color
4. Fix sometimes cannot open videos from system's file manager

### version 3.2 (2020.06.20)

1. Add Classic UI mode
2. Fix false video resolution when using a custom video aspect
3. Fix taskbar icon on Linux ([#88](https://github.com/coslyk/moonplayer/pull/88))
4. Update MPV's opengl-cb to render-api
5. AppImage: Fix ffmpeg not bundled

### version 3.1 (2020.02.08)

1. Add video options and subtitle & audio track selections
2. UI: Use material design
3. UI: Add dark and light theme
4. UI: Use modern dialog to select episodes and video profiles
5. UI: Able to use system's window frame
6. Downloader: Fix no danmaku when opening files from downloader

### version 3.0 (2020.01.03)

1. Program rewritten in QML
2. Re-designed UI
3. Use Javascript for plugins, instead of python

### version 2.9 (2019.10.23)

1. Add classic UI
2. The equalizer's values can be more precise now

### version 2.8 (2019.08.07)

1. Add option: use proxy only for parsing
2. Add socks5 proxy support for hls downloader and ykdl
3. Add support for parsing playlist url
4. Fix: The plugins path for FreeBSD
5. Fix: Use proxy on local hosts
6. Other bug fixes and improvements

### version 2.7 (2019.04.28)

1. Make dragging progressbar smoother
2. Able to remember video quality selection now
3. Add an option to choose the action when opening an URL

### version 2.6 (2019.03.13)

1. Support Windows
2. Use python script to upgrade plugins instead of shell
3. Fix: Wrong sequence when joining more than 10 videos
4. Fix: Danmaku2ass crashes on Python3.7

### version 2.5.2 (2019.02.09)

1. Fix: Cannot open files in some cases, e.g. files out of the Flatpak's sandbox
2. Don't download plugins before MoonPlayer window is shown (seems to cause crash on some platform)
3. Use Python3 on Linux
4. Don't quit when failing to loading plugins
5. Better wayland support
6. Many other code level restructions to make program stabler

### version 2.5 (2019.01.27)

1. Remind users to relaunch MoonPlayer after updating plugins
2. Replace the unmaintained QWebkit with QWebEngine
3. Fix: Cannot play video from dilidili

### version 2.4 (2019.01.04)

1. Replace you-get with youtube-dl
2. Automatically select video parser
3. Show a message when parsing starts
4. Fix: Playing a VIP video may cause crash
5. Fix: some HLS streams cannot be downloaded

### version 2.3 (2018.12.02)

1. Add: Page loading simulating parser
2. Add: Download and update plugins automatically
3. Change default User-Agent
4. Move all plugins to user's directory

### version 2.2 (2018.11.17)

1. Add ARM platform support
2. Change the default paths for downloading and screenshot
3. Close downloader window after opening videos from downloader
4. Handle "moonplayer" and "moonplayers" scheme
5. Fix: Error dialog may be shown twice after parsing video fails

### version 2.1 (2018.11.06)

1. Able to get danmaku from ykdl
2. Better error output

### version 2.0 (2018.10.06)

1. Add: Unblock Chinese websites from oversea

### version 1.9.2 (2018.09.28)

1. Add about dialog

### version 1.9 (2018.09.09)

1. Exit fullscreen by pressing Esc button
2. Able to download HLS stream format

### version 1.8.2 (2018.07.29)

1. Detect OpenGL backend to fix the black screen on Nvidia platforms

### version 1.8 (2018.07.26)

1. Fix: Unable to open videos from download manager
2. Add combined video to playlist automatically

### version 1.7.2 (2018.06.08)

1. Use Qt's builtin HiDPI Scaling to fix the problem with video output

### version 1.7 (2018.05.28)

1. Update youku plugin
2. Automatically use another video parser when one fails to parse

### version 1.6 (2018.03.06)

1. Update youku plugin
2. Run all parser upgrader at a time

### version 1.5 (2018.01.30)

1. Add ykdl video parser

### version 1.4 (2018.01.21)

1. Support external audios
2. Support youtube high quality videos

### version 1.3.2 (2017.12.01)

1. Fix compatibility with the newest version of you-get

### version 1.3 (2017.10.15)

1. Fix: Cannot check newest you-get version in some cases
2. Fix: Wrone key event in some case
3. Fix: mouse cursor hides abnormally
4. Fully support HiDPI

### version 1.2 (2017.09.22)

1. Add youtube plugin
2. Support socks5 proxy
3. Support http proxy in online playing
4. Show tooltip in online video items
5. Able to resize window by dragging corners
6. Set cache size automatically

### version 1.1.7 (2017.09.06)

1. Fix: Cannot download you-get on some platforms
2. Video: Add copy mode

### version 1.1.5 (2017.08.27)

1. Fix: Cannot open https stream in command

### version 1.1 (2017.08.22)

1. Add video equalizer
2. Support choosing and adding subtitles
3. Support dragging subtitle files
4. Support choosing audio channels and tracks
5. Support audio and subtitle's delay
6. Fix: Unable to play Sohu's video online

### version 1.0 (2017.08.15)

1. Rewrite UI
2. Playback switch to opengl-cb
3. Add some new keyboard shortcuts
