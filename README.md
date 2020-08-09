# MoonPlayer

MoonPlayer is an interesting player that lets you to enjoy videos. It can play the video online, download it or just open the local videos.

***

### Homepage

The homepage of MoonPlayer is here: https://coslyk.github.io/moonplayer.html

Here is the development page of this project. For the program usage information, please visit the homepage.

### Compile

Following packages are essential for compiling MoonPlayer.

On ArchLinux:

```
    - cmake
    - ffmpeg
    - mpv
    - python
    - qt5-base
    - qt5-declarative
    - qt5-quickcontrols
    - qt5-quickcontrols2
    - qt5-tools
    - qt5-x11extras
    - wget / curl
```

On Debian:

```
For building:
    - build-essential
    - cmake
    - qtbase5-dev
    - qtbase5-private-dev
    - qtdeclarative5-dev
    - qttools5-dev
    - libqt5x11extras5-dev
    - libmpv-dev
For running:
    - ffmpeg
    - libmpv1
    - libqt5widgets5
    - libqt5network5
    - libqt5x11extras5
    - python
    - qml-module-qtquick2
    - qml-module-qtquick-controls2
    - qml-module-qtquick-dialogs
    - qml-module-qtquick-layouts
    - qml-module-qtquick-window2
    - qml-module-qt-labs-folderlistmodel
    - qml-module-qt-labs-settings
    - wget / curl
```

Other Linux: Please diy.

Download the source code, then run:

```bash
cmake -DCMAKE_INSTALL_PREFIX=/usr .
make
sudo make install
```
