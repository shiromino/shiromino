# Building from source
The following table displays a set of build definitions. To set them, you can specify `-D${OPTION_NAME}=${VALUE}` as a build flag for each definition that you want to enable in the CMake configuration step (which is the first CMake command you run).

For instance, if you wanted to disable the `ENABLE_OPENGL_INTERPOLATION` option, the flag that you would need to provide would be `-DENABLE_OPENGL_INTERPOLATION=0`.
## Build definitions
| Definition                    | Values                                               | Description                                            |
| ----------------------------- | ---------------------------------------------------- | ------------------------------------------------------ |
| [`CMAKE_BUILD_TYPE`][]        | `Debug`, `Release`, `RelWithDebInfo`, `MinSizeRel`    | Controls the type of build that the build process will output.
| [`CMAKE_INSTALL_PREFIX`][]    | any path                                              | When installing the project via `cmake --install`, this prefix will be prepended to all files that the installation process emits. In practice, this allows you to control the installation directory.
| [`CMAKE_TOOLCHAIN_FILE`][]    | any path                                              | When building the project on Windows using `vcpkg`, CMake needs to know where it can find your installed packages. That's why `vcpkg` installations come with a toolchain file that you must specify here.
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
$ brew install cmake libvorbis pkg-config sdl2 sdl2_image sdl2_mixer
$ cmake -B build -S . && cmake --build build -j$(sysctl -n hw.ncpu)
```
In case you're faced with a package-related error after running the `cmake` command, you're likely using an outdated `brew` and need to update first:
```shell
$ brew update
$ brew upgrade pkg-config
```
### Windows (Visual Studio, x64, via cmd.exe or PowerShell)
Before running these instructions, make sure to install [CMake](https://cmake.org/download/), [vcpkg](https://github.com/Microsoft/vcpkg), and [Visual Studio](https://visualstudio.microsoft.com/downloads/). You must add the `vcpkg` installation directory to your `Path` environment variable to be able to use the command in the way we're using it below. Instead of doing that, you could also change into the `vcpkg` installation directory and run the `vcpkg` command there. But if you do, make sure to specify a valid path to `vcpkg-response-file`.

```shell
$ vcpkg install --triplet x64-windows @vcpkg-response-file
$ cmake -B build -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake -S .
$ cmake --build build -j --config Release
```
### Windows (MSYS2, x64)
Before running these instructions, make sure to install [MSYS2](https://www.msys2.org/). If during linking, you see errors about the linker not being able to find `-lmingw32`, reboot your computer before trying again. The following commands need to be run in a "MSYS2 MinGW 64-bit" shell.

```shell
$ pacman --needed --noconfirm -S make mingw-w64-x86_64-{cmake,dlfcn,gcc,glew,libvorbis,SDL2{,_image,_mixer},sqlite3}
$ cmake -B build -G "MSYS Makefiles" -S . && cmake --build build -j$(nproc)
```
<a name="running"></a>
# Running
Usually, you can find your compiled game executable in `./build/shiromino`. Note that the executable may have a native file extension, so expect it to end with `.exe` on Windows. If you've built the game with Visual Studio, the executable is put into `.\build\Release\shiromino.exe`. For the rest of this section, we're going to assume that your executable is located at `./build/shiromino`.

Note: If you used the MSYS2 build instructions, note that double-clicking the executable won't work unless you provide all the DLL files in the same directory. Be advised to start shiromino through the command line as outlined above.

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