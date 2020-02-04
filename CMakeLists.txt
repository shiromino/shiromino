cmake_minimum_required(VERSION 3.10)

project(Shiromino VERSION 0.1 LANGUAGES CXX)

if(VCPKG_TOOLCHAIN)
    set(VCPKG_CRT_LINKAGE dynamic)
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(sdl2 REQUIRED)
find_package(sdl2-image REQUIRED)
find_package(sdl2-mixer REQUIRED)
find_package(ogg REQUIRED)
find_library(VORBIS vorbis)
find_library(VORBISFILE vorbisfile)
find_package(sqlite3 REQUIRED)

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
    src/SGUIL/SGUIL.cpp
)

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
    src/PCG/pcg_uint128.hpp
)
source_group("Header Files" FILES ${HEADERS})

add_executable(Shiromino ${SOURCES} ${HEADERS})

# For whatever reason, the proper include directory for sqlite3 isn't set when
# using vcpkg and Visual Studio, so the include directory has to be manually
# added.
if(VCPKG_TOOLCHAIN)
    find_path(SYSTEM_INCLUDE_DIR sqlite3.h)
else()
    set(SYSTEM_INCLUDE_DIR "")
endif()

target_include_directories(Shiromino PUBLIC ${PROJECT_BINARY_DIR} PRIVATE ${SYSTEM_INCLUDE_DIR})

set(CMAKE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(Shiromino PRIVATE
    SDL2::SDL2 SDL2::SDL2main
    SDL2::SDL2_image
    SDL2::SDL2_mixer
    SQLite::SQLite3
    Ogg::ogg
    ${VORBIS}
    ${VORBISFILE}
)
