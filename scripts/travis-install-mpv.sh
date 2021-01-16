#!/bin/sh

# Download libmpv
curl -Lo libmpv.7z https://deac-ams.dl.sourceforge.net/project/mpv-player-windows/libmpv/mpv-dev-x86_64-20210103-git-3e175df.7z

# Extract libmpv
[ -d libmpv ] || mkdir libmpv
7z x -olibmpv libmpv.7z

# Produce .lib file
cd libmpv
mv include mpv
lib /def:mpv.def /name:mpv-1.dll /out:mpv.lib /MACHINE:X64
cd ..