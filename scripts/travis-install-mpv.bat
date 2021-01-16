
: Download libmpv
curl -Lo libmpv.7z https://deac-ams.dl.sourceforge.net/project/mpv-player-windows/libmpv/mpv-dev-x86_64-20210103-git-3e175df.7z

: Extract libmpv
7z x -olibmpv libmpv.7z

: Set up Visual C++ Environment
: https://travis-ci.community/t/how-to-c-in-windows/3273/4
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\Common7\Tools\VsDevCmd.bat" -host_arch=amd64 -arch=amd64
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\Common7\Tools\VsDevCmd.bat" -test

: Produce .lib file
cd libmpv
rename include mpv
lib /def:mpv.def /name:mpv-1.dll /out:mpv.lib /MACHINE:X64
cd ..