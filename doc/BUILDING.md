# Building from source
The following table displays a set of build definitions. To set them, you can specify `-D${OPTION_NAME}=${VALUE}` as a build flag for each definition that you want to enable in the CMake configuration step (which is the first CMake command you run).

For instance, if you wanted to disable the `ENABLE_OPENGL_INTERPOLATION` option, the flag that you would need to provide would be `-DENABLE_OPENGL_INTERPOLATION=0`.
## Build definitions
| Definition                    | Values                                               | Description                                            |
| ----------------------------- | ---------------------------------------------------- | ------------------------------------------------------ |
| [`CMAKE_BUILD_TYPE`][]        | `Debug`, `Release`, `RelWithDebInfo`, `MinSizeRel`    | Controls the type of build that the build process will output.
| [`CMAKE_INSTALL_PREFIX`][]    | any path                                              | When installing the project via `cmake --install`, this prefix will be prepended to all files that the installation process emits. In practice, this allows you to control the installation directory.
| `ENABLE_OPENGL_INTERPOLATION` | `0`, `1`                                            | Enables support for the `INTERPOLATE` option in `shiromino.ini`, which works best when combined with the video stretch option. Note that this definition requires OpenGL 3.3 Core Profile support. This definition defaults to `1` if OpenGL can be found on your system, unless you're using macOS, as it has deprecated OpenGL.
## Installing dependencies and compiling
In the following, please follow the steps that match your build environment. All of the sections below assume that your current working directory is the repository's root directory.
### Linux (pacman-based)
```shell
$ sudo pacman --needed -S gcc cmake sdl2 sdl2_image sdl2_mixer libvorbis sqlite
$ cmake -B build -S . && cmake --build build -j$(nproc)
```
### Linux (apt-based)
```shell
$ sudo apt install build-essential cmake libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libvorbis-dev libsqlite3-dev
$ cmake -B build -S . && cmake --build build -j$(nproc)
```
### Linux (rpm-based)
```shell
$ sudo dnf install cmake gcc-c++ SDL2-devel SDL2_image-devel SDL2_mixer-devel libvorbis-devel libsq3-devel
$ cmake -B build -S . && cmake --build build -j$(nproc)
```
### macOS
Before running these instructions, make sure to install [Homebrew](https://brew.sh/).
```shell
$ brew install cmake libvorbis sdl2 sdl2_image sdl2_mixer
$ cmake -B build -S . && cmake --build build -j$(sysctl -n hw.ncpu)
```
In case you're faced with a package-related error after running the `cmake` command, you're likely using an outdated `brew` and need to update first:
```shell
$ brew update
```
### Windows
Before running these instructions, make sure to install [MSYS2](https://www.msys2.org/). The following commands need to be run in a "MSYS2 MinGW 64-bit" shell.

```shell
$ pacman --needed --noconfirm -S make mingw-w64-x86_64-{cmake,dlfcn,gcc,glew,libvorbis,SDL2{,_image,_mixer},sqlite3}
$ cmake -B build -G "MSYS Makefiles" -S . && cmake --build build -j$(nproc)
```
<a name="running"></a>
# Running the game
You can find your compiled game executable in `./build/shiromino`. Note that the executable may have a native file extension, so expect it to end with `.exe` on Windows.

Note that if you're on Windows and you've built shiromino from source, double-clicking the executable won't work unless you've put all the dependent DLL files into the build directory. To remedy this, be advised to run shiromino through the command line. Alternatively, you could just grab one of our pre-built [Windows releases](https://github.com/shiromino/shiromino/releases) so that you don't have to compile the game from source.

In order to run the game, just launch the executable. Please beware a successful launch is tied to a successful [path resolution](RUNNING.md#path-resolution).
```shell
$ ./build/shiromino
```
# Installing and packaging
This game can be installed through CMake. Please beware that currently, Linux is the only supported platform. In CMake's configuration step, you can set a custom installation path via [`CMAKE_INSTALL_PREFIX`][]. Note that in order to install shiromino, you must have built it first.
```shell
$ cmake --install build
```
Installing the game via CMake will create a hierarchy of [`GNUInstallDirs`][] in [`CMAKE_INSTALL_PREFIX`][] that should be ideal for packaging this game.

[`CMAKE_BUILD_TYPE`]: https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html
[`CMAKE_INSTALL_PREFIX`]: https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX.html
[`CMAKE_TOOLCHAIN_FILE`]: https://cmake.org/cmake/help/latest/variable/CMAKE_TOOLCHAIN_FILE.html
[`GNUInstallDirs`]: https://cmake.org/cmake/help/v3.0/module/GNUInstallDirs.html