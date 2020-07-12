# shiromino
<a href="https://github.com/nightmareci/shiromino/actions" rel="Build status">![build](https://github.com/nightmareci/shiromino/workflows/build/badge.svg)</a>
## Building
The following table displays a set of build options. To enable them, you can specify `-D${OPTION_NAME}=${VALUE}` as a build flag for each option that you want to enable in the CMake configuration step (which is the first CMake command you run).

For instance, if you wanted to enable the `OPENGL_INTERPOLATION` option, the flag that you would need to provide would be `-DOPENGL_INTERPOLATION=1`.
### Build options
| Option                 | Values    | Description                                                                              |
| ---------------------- | --------- | ---------------------------------------------------------------------------------------- |
| `OPENGL_INTERPOLATION` | `0`, `1`  | Enables (`1`) or disables (`0`) support for the `INTERPOLATE` option in `game.ini`, which works best when combined with the video stretch option. Note that this option requires SDL2 ≥ 2.0.10 and OpenGL 3.3 Core Profile support.
### Installing dependencies and compiling
In order to build this project, you will need a C++17 compiler, CMake (≥ 3.10), SDL2 (≥ 2.0.5), SDL2_image, SDL2_mixer, libvorbis and SQLite3. In the following, please follow the steps that match your build environment. All of the sections below assume that your current working directory is the repository's root directory.
#### Linux (Arch-Linux-based)
```shell
$ sudo pacman -S gcc cmake sdl2 sdl2_image sdl2_mixer libvorbis sqlite
$ cmake -B build -S . && cmake --build build -j$(nproc)
```
#### Linux (Debian-based)
```shell
$ sudo apt install build-essential cmake libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libvorbis-dev libsqlite3-dev
$ cmake -B build -S . && cmake --build build -j$(nproc)
```
#### macOS
```shell
$ brew install cmake libvorbis pkg-config sdl2 sdl2_image sdl2_mixer sqlite
$ cmake -B build -S . && cmake --build build -j$(sysctl -n hw.ncpu)
```
#### Windows (Visual Studio, x64)
Before running these instructions, make sure to install [CMake](https://cmake.org/download/), [vcpkg](https://github.com/Microsoft/vcpkg), and [Visual Studio](https://visualstudio.microsoft.com/downloads/). You may have to add the `vcpkg` installation directory to your `Path` environment variable to be able to use the command in this fashion. Instead, you can also change into the `vcpkg` installation directory and run the `vcpkg` command there.

```shell
$ vcpkg install --triplet x64-windows libvorbis sdl2 sdl2-image sdl2-mixer[dynamic-load,libvorbis] sqlite3
$ cmake -B build -S . && cmake --build build -j
```
#### Windows (MSYS2, x64)
Before running these instructions, make sure to install [MSYS2](https://www.msys2.org/). If during linking, you see errors about the linker not being able to find `-lmingw32`, reboot your computer before trying again. The following commands need to be run in a "MSYS2 MinGW 64-bit" shell.

```shell
$ pacman --needed --noconfirm -S cmake gcc make mingw-w64-x86_64-{dlfcn,libvorbis,SDL2{,_image,_mixer}} sqlite3
$ cmake -B build -G "MSYS Makefiles" -S . && cmake --build build -j$(nproc)
```
## Running
You can find your compiled executable in `./build/Shiromino`. Note that the executable may have a native file extension, so expect it to end with `.exe` on Windows.

To ensure that the game finds its assets, make sure to use the project root directory as your current working directory when running the game, or use `game.ini` to set `BASE_PATH` accordingly. The easiest way to check if everything works should be:

```shell
$ ./build/Shiromino
```

Note: If you used the MSYS2 build instructions, note that double-clicking the executable won't work unless you provide all the DLL files in the same directory. Be advised to start shiromino through the command line as outlined above.