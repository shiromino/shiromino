# shiromino
<a href="https://github.com/shiromino/shiromino/actions" rel="Build status">![build](https://github.com/shiromino/shiromino/workflows/build/badge.svg)</a>
## Building
The following table displays a set of build definitions. To set them, you can specify `-D${OPTION_NAME}=${VALUE}` as a build flag for each definition that you want to enable in the CMake configuration step (which is the first CMake command you run).

For instance, if you wanted to disable the `ENABLE_OPENGL_INTERPOLATION` option, the flag that you would need to provide would be `-DENABLE_OPENGL_INTERPOLATION=0`.
### Build definitions
| Definition                    | Values    | Description                                                                         |
| ----------------------------- | --------- |  -----------------------------------------------------------------------------------|
| `ENABLE_OPENGL_INTERPOLATION` | `0`, `1`  | Enables support for the `INTERPOLATE` option in `game.ini`, which works best when combined with the video stretch option. Note that this definition requires SDL2 ≥ 2.0.10 and OpenGL 3.3 Core Profile support. This definition defaults to `1` if OpenGL can be found on your system.
### Installing dependencies and compiling
In order to build this project, you will need a C++17 compiler, CMake (≥ 3.15), SDL2 (≥ 2.0.5), SDL2_image, SDL2_mixer, libvorbis, and SQLite3. In the following, please follow the steps that match your build environment. All of the sections below assume that your current working directory is the repository's root directory.
#### Linux (Arch-Linux-based)
```shell
$ sudo pacman --needed -S gcc cmake sdl2 sdl2_image sdl2_mixer libvorbis sqlite
$ cmake -B build -S . && cmake --build build -j$(nproc)
```
#### Linux (Debian-based)
```shell
$ sudo apt install build-essential cmake libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libvorbis-dev libsqlite3-dev
$ cmake -B build -S . && cmake --build build -j$(nproc)
```
#### macOS
Before running these instructions, make sure to install [Homebrew](https://brew.sh/).
```shell
$ brew install cmake libvorbis pkg-config sdl2 sdl2_image sdl2_mixer
$ cmake -B build -S . && cmake --build build -j$(sysctl -n hw.ncpu)
```
In case you're faced with a package-related error after running the `cmake` command, you're likely using an outdated `brew` and need to update first:
```shell
$ brew update
$ brew upgrade pkg-config
```
#### Windows (Visual Studio, x64, via cmd.exe or PowerShell)
Before running these instructions, make sure to install [CMake](https://cmake.org/download/), [vcpkg](https://github.com/Microsoft/vcpkg), and [Visual Studio](https://visualstudio.microsoft.com/downloads/). You may have to add the `vcpkg` installation directory to your `Path` environment variable to be able to use the command in this fashion. Instead, you can also change into the `vcpkg` installation directory and run the `vcpkg` command there.

```shell
$ vcpkg install --triplet x64-windows libvorbis opengl sdl2 sdl2-image sdl2-mixer[dynamic-load,libvorbis] sqlite3
$ cmake -B build -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake -S .
$ cmake --build build -j --config Release
```
#### Windows (MSYS2, x64)
Before running these instructions, make sure to install [MSYS2](https://www.msys2.org/). If during linking, you see errors about the linker not being able to find `-lmingw32`, reboot your computer before trying again. The following commands need to be run in a "MSYS2 MinGW 64-bit" shell.

```shell
$ pacman --needed --noconfirm -S cmake gcc make mingw-w64-x86_64-{dlfcn,libvorbis,glew,SDL2{,_image,_mixer}} sqlite3
$ cmake -B build -G "MSYS Makefiles" -S . && cmake --build build -j$(nproc)
```
## Running
Usually, you can find your compiled game executable in `./build/shiromino`. Note that the executable may have a native file extension, so expect it to end with `.exe` on Windows. If you've built the game with Visual Studio, the executable is put into `.\build\Release\shiromino.exe`. For the rest of this section, we're going to assume that your executable is located at `./build/shiromino`.

Note: If you used the MSYS2 build instructions, note that double-clicking the executable won't work unless you provide all the DLL files in the same directory. Be advised to start shiromino through the command line as outlined above.

Besides the game executable, the game needs a few files in order to run properly. For one, there's `game.ini`, a configuration file that you can use to specify key bindings and other settings. Then, there's also the `assets` directory which includes audio and image files.

There are two ways to make the game find these files. The first way is to just put them into the same directory as the `shiromino` executable. After doing that, you can run the game via:
```shell
$ ./build/shiromino
```
Especially during development, this approach is very inflexible. A second way is to specify a path to the configuration file (`game.ini`) with `--configuration-file` or `-c` on the command line. For example, you could provide the flag like this from the source directory:
```shell
$ ./build/shiromino --configuration-file ./game.ini
```
The path to `assets` is resolved in the following order:
- If `--configuration-file` is left unspecified, try `<directory of executable>/assets`.
- Otherwise, if the configuration file sets `BASE_PATH`, try `${BASE_PATH}/assets`.
- Otherwise, try: `<directory of configuration file>/assets`.