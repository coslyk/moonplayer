: Download libmpv
curl -Lo libmpv.7z https://udomain.dl.sourceforge.net/project/mpv-player-windows/libmpv/mpv-dev-x86_64-20230611-git-1c82d6a.7z

: Extract libmpv
7z x -olibmpv libmpv.7z

: Produce .lib file
cd libmpv
rename mpv.def mpv.def.bak
echo EXPORTS > mpv.def
type mpv.def.bak >> mpv.def
lib /def:mpv.def /name:libmpv-2.dll /out:mpv.lib /MACHINE:X64
cd ..
