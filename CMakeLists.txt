cmake_minimum_required(VERSION 3.10)

project(Shiromino VERSION 0.1 LANGUAGES CXX)

if(${VCPKG_TOOLCHAIN})
    set(VCPKG_CRT_LINKAGE dynamic)
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()

if(${VCPKG_TOOLCHAIN})
    # Some features supported by GCC for C++17 aren't supported by MSVC
    # C++17. This makes MSVC use the "latest" standard, which has the
    # features we need.
    set(CMAKE_CXX_STANDARD 20)
else()
    # For now, only C++17 features will be used, despite needing to use
    # C++20 for MSVC.
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig)
if(${PKG_CONFIG_FOUND})
    pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
    pkg_check_modules(SDL2_image REQUIRED IMPORTED_TARGET SDL2_image)
    pkg_check_modules(SDL2_mixer REQUIRED IMPORTED_TARGET SDL2_mixer)
    pkg_check_modules(sqlite3 REQUIRED IMPORTED_TARGET sqlite3)
    set(LIBRARIES
        SDL2
        SDL2_image
        SDL2_mixer
        sqlite3)
else()
    find_package(sdl2 REQUIRED)
    find_package(sdl2-image REQUIRED)
    find_package(sdl2-mixer REQUIRED)
    find_package(sqlite3 REQUIRED)
    if(${VCPKG_TOOLCHAIN})
        # vcpkg doesn't automatically copy over the OGG/Vorbis libraries needed
        # to play OGG files with SDL_mixer, so they have to be explicitly added
        # as dependencies to Shiromino to work around that bug.
        find_package(ogg REQUIRED)
        find_library(VORBIS_LIBRARY vorbis)
        find_library(VORBISFILE_LIBRARY vorbisfile)
        set(LIBRARIES
            SDL2::SDL2 SDL2::SDL2main
            SDL2::SDL2_image
            SDL2::SDL2_mixer
            SQLite::SQLite3
            ${OGG_LIBARIES}
            ${VORBIS_LIBRARY}
            ${VORBISFILE_LIBRARY})

        add_compile_definitions(VCPKG_TOOLCHAIN)
    else()
        set(LIBRARIES
            SDL2::SDL2 SDL2::SDL2main
            SDL2::SDL2_image
            SDL2::SDL2_mixer
            SQLite::SQLite3)
    endif()
# TODO: Other platforms; macOS for sure.
endif()

configure_file(include/Config.hpp.in Config.hpp)

set(SOURCES
    src/Audio.cpp
    src/bstrlib.cpp
    src/core.cpp
    src/file_io.cpp
    src/game_menu.cpp
    src/game_qs.cpp
    src/gfx.cpp
    src/gfx_helpers.cpp
    src/gfx_menu.cpp
    src/gfx_qs.cpp
    src/grid.cpp
    src/GuiGridCanvas.cpp
    src/GuiScreenManager.cpp
    src/main.cpp
    src/MultiEditor.cpp
    src/piecedef.cpp
    src/qrs.cpp
    src/random.cpp
    src/replay.cpp
    src/rotation_tables.cpp
    src/scores.cpp
    src/ShiroPhysoMino.cpp
    src/SPM_Randomizer.cpp
    src/SPM_Spec.cpp
    src/timer.cpp

    src/SGUIL/SGUIL_GuiButton.cpp
    src/SGUIL/SGUIL_GuiDropDownList.cpp
    src/SGUIL/SGUIL_GuiOptionButton.cpp
    src/SGUIL/SGUIL_GuiTextField.cpp
    src/SGUIL/SGUIL_GuiWindow.cpp
    src/SGUIL/SGUIL.cpp)

set(HEADERS
    src/Audio.hpp
    src/bstr_to_std.hpp
    src/bstrlib.h
    src/core.h
    src/Debug.hpp
    src/file_io.h
    src/fonts.h
    src/game_menu.h
    src/game_qs.h
    src/gfx.h
    src/gfx_helpers.hpp
    src/gfx_menu.h
    src/gfx_qs.h
    src/gfx_structures.h
    src/grid.h
    src/GuiGridCanvas.hpp
    src/GuiScreenManager.hpp
    src/images.h
    src/MultiEditor.hpp
    src/music.h
    src/PDINI.hpp
    src/piecedef.h
    src/player.h
    src/qrs.h
    src/QRS.hpp
    src/random.h
    src/replay.h
    src/rotation_tables.h
    src/scores.h
    src/sfx.h
    src/ShiroPhysoMino.hpp
    src/SPM_Randomizer.hpp
    src/SPM_Spec.hpp
    src/stringtools.hpp
    src/TGM.hpp
    src/timer.h

    src/SGUIL/SGUIL.hpp

    src/PCG/pcg_extras.hpp
    src/PCG/pcg_random.hpp
    src/PCG/pcg_uint128.hpp)
source_group("Header Files" FILES ${HEADERS})

add_executable(Shiromino ${SOURCES} ${HEADERS})

if(${VCPKG_TOOLCHAIN})
    # For whatever reason, the proper include directory for sqlite3 isn't set when
    # using vcpkg and Visual Studio, so the include directory has to be manually
    # added.
    find_path(INCLUDE_DIRS sqlite3.h)
elseif(${PKG_CONFIG_FOUND})
    target_compile_options(Shiromino PUBLIC ${SDL2_CFLAGS_OTHER} ${SDL2_IMAGE_CFLAGS_OTHER} ${SDL2_MIXER_CFLAGS_OTHER} ${SQLITE3_CFLAGS_OTHER})
    set(INCLUDE_DIRS ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS} ${SQLITE3_INCLUDE_DIRS})
endif()

target_include_directories(Shiromino PUBLIC ${PROJECT_BINARY_DIR} PRIVATE ${INCLUDE_DIRS})

set(CMAKE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(Shiromino PRIVATE ${LIBRARIES})
