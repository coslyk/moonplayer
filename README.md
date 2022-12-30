# MoonPlayer
[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2Fcoslyk%2Fmoonplayer.svg?type=shield)](https://app.fossa.com/projects/git%2Bgithub.com%2Fcoslyk%2Fmoonplayer?ref=badge_shield)


MoonPlayer is an interesting player that lets you to enjoy videos. It can play the video online, download it or just open the local videos.

***

## Homepage

The homepage of MoonPlayer is here: https://coslyk.github.io/moonplayer.html

Here is the development page of this project. For the introduction and usage information, please visit the homepage.

[Update log](https://github.com/coslyk/moonplayer/blob/develop/NEWS.md)

## Installation

#### Windows / macOS / Linux (AppImage)

Download from [GitHub Releases](https://github.com/coslyk/MoonPlayer/releases) and install it.

**Note:**
1. Windows version only support x64.
2. AppImage version does not support hardware decoding.

#### Linux (Flatpak)

Install from [Flathub](https://flathub.org/apps/details/com.github.coslyk.MoonPlayer): `flatpak install flathub com.github.coslyk.MoonPlayer`

#### Linux (Debian)

Add [DebianOpt](https://github.com/coslyk/debianopt-repo) repository, then install with `sudo apt install moonplayer`.

#### Linux (ArchLinux, Manjaro)

Add [ArchLinuxCN](https://www.archlinuxcn.org/archlinux-cn-repo-and-mirror/) repository, then install with `pacman -S moonplayer`.

## Screenshot

![](https://coslyk.github.io/files/moonplayer-play.png)

## Development

Following packages are essential for compiling MoonPlayer.

On ArchLinux:

```
    - cmake
    - ffmpeg
    - mpv
    - python
    - qt5-base
    - qt5-declarative
    - qt5-quickcontrols2
    - qt5-tools
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
    - libmpv-dev
For running:
    - ffmpeg
    - libmpv1
    - libqt5network5
    - python
    - qml-module-qtquick2
    - qml-module-qtquick-controls2
    - qml-module-qtquick-layouts
    - qml-module-qtquick-window2
    - qml-module-qt-labs-folderlistmodel
    - qml-module-qt-labs-settings
    - wget / curl
```

Other Linux: Please diy.

Download the source code, then run:

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

**Note:** MoonPlayer uses Qt's private API, so you may need to re-compile MoonPlayer after Qt is upgraded.

## Technology stack

- [Qt](https://www.qt.io/) (License: LGPL-3)

- [libmpv](https://mpv.io/) (License: GPLv2+)

- [ffmpeg](https://ffmpeg.org/) (License: GPLv2+)

- [yk-dlp](https://github.com/yt-dlp/yt-dlp) (License: Unlicense)

- [lux](https://github.com/iawia002/lux) (License: MIT)

- [hlsdl](https://github.com/selsta/hlsdl) (License: MIT)

- [Danmaku2Ass-Cpp](https://github.com/coslyk/danmaku2ass_cpp) (License: WTFPL)

## License

[GPLv3+](https://github.com/coslyk/moonplayer/blob/develop/LICENSE)

[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2Fcoslyk%2Fmoonplayer.svg?type=large)](https://app.fossa.com/projects/git%2Bgithub.com%2Fcoslyk%2Fmoonplayer?ref=badge_large)