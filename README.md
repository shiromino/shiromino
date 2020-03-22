# Shiromino

## Building
Dependencies: CMake, SDL2 (version >= 2.0.5), SDL2_image, SDL2_mixer, OGG,
vorbis, vorbisfile, SQLite3

Create a directory to build in somewhere, change to that directory, run `cmake
<shiromino-source-base-directory>` in the build directory, then build with the
build tool on your platform (such as `make`). When building on Windows with
Visual Studio, use [Vcpkg](https://github.com/Microsoft/vcpkg) for installing
the libraries and use the official [CMake Windows
distribution](https://cmake.org/download/).
