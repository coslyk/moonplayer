: Download libmpv
curl -Lo libmpv.7z https://udomain.dl.sourceforge.net/project/mpv-player-windows/libmpv/mpv-dev-x86_64-20220821-git-37aea11.7z

: Extract libmpv
7z x -olibmpv libmpv.7z

: Produce .lib file
cd libmpv
rename include mpv
lib /def:mpv.def /name:mpv-2.dll /out:mpv.lib /MACHINE:X64
cd ..