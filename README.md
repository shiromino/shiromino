# Shiromino

## Building
Dependencies: Compiler supporting C++17, CMake (version >= 3.10), SDL2,
SDL2_image, SDL2_mixer, OGG, vorbis, vorbisfile, SQLite3

SDL2 2.0.5 is required when not building with optional features. SDL2 2.0.10 is
required when building with interpolation enabled.

Create a directory to build in somewhere, change to that directory, run `cmake
<shiromino-source-base-directory>` in the build directory, then build with the
build tool on your platform (such as `make`). When building on Windows with
Visual Studio, use [Vcpkg](https://github.com/Microsoft/vcpkg) for installing
the libraries and use the official [CMake Windows
distribution](https://cmake.org/download/). You can also build on Windows in an
[MSYS2](https://www.msys2.org/) environment.

There's a CMake variable, OPENGL_INTERPOLATION, that when set, enables support
for an interpolated scaling INI option, best when combined with the video
stretch option; interpolation isn't compiled in by default, and for now
requires OpenGL 3.3 Core Profile support.

## Building and running on Windows with Visual Studio and Vcpkg
TODO

## Building and running on Linux or similar command line environments
```shell
# Install packages on Debian-based distributions.
# GCC 7 or newer (g++-7, g++-8, etc. for Debian) is required, for C++17 language support.
$ sudo apt install g++-7 cmake libsdl2-devel libsdl2-image-dev libsdl2-mixer-dev libogg-dev libvorbis-dev libsqlite3-dev

# Install packages on Arch Linux-based (Manjaro, etc.) distributions.
$ sudo pacman -S gcc cmake sdl2 sdl2_image sdl2_mixer libogg libvorbis sqlite

# Install packages in a Windows MSYS2 64-bit environment.
$ pacman -S gcc cmake mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image mingw-w64-x86_64-SDL2_mixer mingw-w64-x86_64-libogg  mingw-w64-x86_64-libvorbis sqlite3

$ cd shiromino
$ mkdir build
$ cd build

# Use this for Linux.
$ cmake ..
# Enable support for the interpolation option.
$ cmake -D OPENGL_INTERPOLATION=1 ..

# Use this on Windows with MSYS2.
$ cmake -G "MSYS Makefiles" ..
# Enable support for the interpolation option.
$ cmake -G "MSYS Makefiles" -D OPENGL_INTERPOLATION=1 ..

# Omit -j$(nproc) if your platform doesn't have nproc.
$ make -j$(nproc)
$ cd ..
$ ./build/Shiromino
```
