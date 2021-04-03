# shiromino
<a href="https://github.com/shiromino/shiromino/actions" rel="Build status">![build](https://github.com/shiromino/shiromino/workflows/build/badge.svg)</a>
## Official packages
Depending on your setup, you might not have to build this game yourself. Instead, consider using one of the following packages.
| Repository                    | Package name
| ----------------------------- | ---------------
| Arch User Repository (AUR)    | [`shiromino-git`](https://aur.archlinux.org/cgit/aur.git/tree/PKGBUILD?h=shiromino-git)
## Building from source
The following table displays a set of build definitions. To set them, you can specify `-D${OPTION_NAME}=${VALUE}` as a build flag for each definition that you want to enable in the CMake configuration step (which is the first CMake command you run).

For instance, if you wanted to disable the `ENABLE_OPENGL_INTERPOLATION` option, the flag that you would need to provide would be `-DENABLE_OPENGL_INTERPOLATION=0`.
### Build definitions
| Definition                    | Values                                               | Description                                            |
| ----------------------------- | ---------------------------------------------------- | ------------------------------------------------------ |
| [`CMAKE_BUILD_TYPE`][]        | `Debug`, `Release`, `RelWithDebInfo`, `MinSizeRel`    | Controls the type of build that the build process will output.
| [`CMAKE_INSTALL_PREFIX`][]    | any path                                              | When installing the project via `cmake --install`, this prefix will be prepended to all files that the installation process emits. In practice, this allows you to control the installation directory.
| [`CMAKE_TOOLCHAIN_FILE`][]    | any path                                              | When building the project on Windows using `vcpkg`, CMake needs to know where it can find your installed packages. That's why `vcpkg` installations come with a toolchain file that you must specify here.
| `ENABLE_OPENGL_INTERPOLATION` | `0`, `1`                                            | Enables support for the `INTERPOLATE` option in `shiromino.ini`, which works best when combined with the video stretch option. Note that this definition requires OpenGL 3.3 Core Profile support. This definition defaults to `1` if OpenGL can be found on your system, unless you're using macOS, as it has deprecated OpenGL.
### Installing dependencies and compiling
In the following, please follow the steps that match your build environment. All of the sections below assume that your current working directory is the repository's root directory.
#### Linux (pacman-based)
```shell
$ sudo pacman --needed -S gcc cmake sdl2 sdl2_image sdl2_mixer libvorbis sqlite
$ cmake -B build -S . && cmake --build build -j$(nproc)
```
#### Linux (apt-based)
```shell
$ sudo apt install build-essential cmake libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libvorbis-dev libsqlite3-dev
$ cmake -B build -S . && cmake --build build -j$(nproc)
```
#### Linux (rpm-based)
```shell
$ sudo dnf install cmake gcc-c++ SDL2-devel SDL2_image-devel SDL2_mixer-devel libvorbis-devel libsq3-devel
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
Before running these instructions, make sure to install [CMake](https://cmake.org/download/), [vcpkg](https://github.com/Microsoft/vcpkg), and [Visual Studio](https://visualstudio.microsoft.com/downloads/). You must add the `vcpkg` installation directory to your `Path` environment variable to be able to use the command in the way we're using it below. Instead of doing that, you could also change into the `vcpkg` installation directory and run the `vcpkg` command there. But if you do, make sure to specify a valid path to `vcpkg-response-file`.

```shell
$ vcpkg install --triplet x64-windows @vcpkg-response-file
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

In order to run the game, just launch the executable. Please beware a successful launch is tied to a successful [path resolution](#path-resolution).
```shell
$ ./build/shiromino
```
<a name="custom-configuration-file"></a>
If you would like to specify a custom configuration file, you can do so with the command-line options `--configuration-file` or `-c`. For example, you could provide the flag like this from the source directory:
```shell
$ ./build/shiromino --configuration-file ./shiromino.ini
```
<a name="path-resolution"></a>
## Path resolution
Besides the game executable, the game needs to resolve the following files or paths in order to run properly.
### `shiromino.ini`
This configuration file is where you specify key bindings and other settings.

If no configuration file is specified, the game will look for this file here (in order):

1. `<executable directory>/shiromino.ini`
2. `<executable directory>/../etc/shiromino.ini`
3. `<current working directory>/shiromino.ini`
4. `$XDG_CONFIG_HOME/shiromino.ini`
5. `$HOME/.config/shiromino.ini`
6. `/usr/local/etc/shiromino.ini`
7. `/etc/shiromino.ini`

The first path that matches will be honored.

If none match, the game will try to create a default configuration file here (in order):

1. `$XDG_CONFIG_HOME/shiromino.ini`
2. `$HOME/.config/shiromino.ini`
3. `<executable directory>/shiromino.ini`

You can also [specify a custom configuration file](#custom-configuration-file).
### `SHARE_PATH`
`SHARE_PATH` is a path to a directory which contains static game data such as assets or the license. The game assumes that there is a directory within `SHARE_PATH` named `data`.

If `SHARE_PATH` is left unspecified, the game will look for it here (in order):

1. `<executable directory>`
2. `<executable directory>/../share/shiromino`
3. `<current working directory>`
4. `$XDG_DATA_HOME/shiromino`
5. `$HOME/.local/share/shiromino`
6. `/usr/local/share/shiromino`
7. `/usr/share/shiromino`

Any path in this list will match only if it contains a directory named `data`.

The first path that matches will be honored.

You can also specify a custom `SHARE_PATH` in the configuration file. If `SHARE_PATH` is relative, the path is assumed to be relative to the configuration file.
### `CACHE_PATH`
`CACHE_PATH` is a path to a directory which contains dynamic game data such as high scores.

If `CACHE_PATH` is left unspecified, the game will look for it here (in order):

1. `<executable directory>`
2. `<executable directory>/../var/cache/shiromino`
3. `<current working directory>`
4. `$XDG_CACHE_HOME/shiromino`
5. `$HOME/.cache/shiromino`
6. `/usr/local/var/cache/shiromino`
7. `/var/cache/shiromino`

Any path in this list will match only if it contains a file named `shiromino.sqlite`.

The first path that matches will be honored.

If none match, the game will try to create the following directories and use them (in order):

1. `$XDG_CACHE_HOME/shiromino`
2. `$HOME/.cache/shiromino`
3. `<executable directory>`

You can also specify a custom `CACHE_PATH` in the configuration file. If `CACHE_PATH` is relative, the path is assumed to be relative to the configuration file.
## Installing and packaging
This game can be installed through CMake. Please beware that currently, Linux is the only supported platform. In CMake's configuration step, you can set a custom installation path via [`CMAKE_INSTALL_PREFIX`][]. Note that in order to install shiromino, you must have built it first.
```shell
$ cmake --install build
```
Installing the game via CMake will create a hierarchy of [`GNUInstallDirs`][] in [`CMAKE_INSTALL_PREFIX`][] that should be ideal for packaging this game.

[`CMAKE_BUILD_TYPE`]: https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html
[`CMAKE_INSTALL_PREFIX`]: https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX.html
[`CMAKE_TOOLCHAIN_FILE`]: https://cmake.org/cmake/help/latest/variable/CMAKE_TOOLCHAIN_FILE.html
[`GNUInstallDirs`]: https://cmake.org/cmake/help/v3.0/module/GNUInstallDirs.html