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

if(${OPENGL_INTERPOLATION})
    find_package(OpenGL REQUIRED)
    set(OPENGL_LIBRARY OpenGL::GL)
else()
    set(OPENGL_LIBRARY "")
endif()

find_package(PkgConfig)
if(${PKG_CONFIG_FOUND})
    pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
    pkg_check_modules(SDL2_image REQUIRED IMPORTED_TARGET SDL2_image)
    pkg_check_modules(SDL2_mixer REQUIRED IMPORTED_TARGET SDL2_mixer)
    pkg_check_modules(sqlite3 REQUIRED IMPORTED_TARGET sqlite3)
    set(LIBRARIES
        ${SDL2_LIBRARIES}
        ${SDL2_image_LIBRARIES}
        ${SDL2_mixer_LIBRARIES}
        ${sqlite3_LIBRARIES}
        ${OPENGL_LIBRARY}
        ${CMAKE_DL_LIBS})
else()
    find_package(sdl2 REQUIRED)
    find_package(sdl2-image REQUIRED)
    find_package(sdl2-mixer REQUIRED)
    find_package(sqlite3 REQUIRED)
    if(${VCPKG_TOOLCHAIN})
        # When using dynamically linked libraries, vcpkg doesn't automatically
        # copy over the OGG/Vorbis libraries needed to play OGG files with
        # SDL_mixer, so they have to be explicitly added as dependencies to
        # Shiromino to work around that bug.
        find_package(ogg REQUIRED)
        find_library(VORBIS_LIBRARY vorbis)
        find_library(VORBISFILE_LIBRARY vorbisfile)
        set(LIBRARIES
            SDL2::SDL2 SDL2::SDL2main
            SDL2::SDL2_image
            SDL2::SDL2_mixer
            SQLite::SQLite3
            ${OPENGL_LIBRARY}
            ${OGG_LIBARIES}
            ${VORBIS_LIBRARY}
            ${VORBISFILE_LIBRARY})

        add_compile_definitions(VCPKG_TOOLCHAIN)
    else()
        set(LIBRARIES
            SDL2::SDL2 SDL2::SDL2main
            SDL2::SDL2_image
            SDL2::SDL2_mixer
            SQLite::SQLite3
            ${OPENGL_LIBRARY})
    endif()
# TODO: Other platforms; macOS for sure.
endif()

configure_file(include/Config.hpp.in Config.hpp @ONLY)

if(${OPENGL_INTERPOLATION})
    set(OPENGL_SOURCES third-party/glad.cpp)
    set(OPENGL_HEADERS
        third-party/glad.h
        third-party/khrplatform.h)
else()
    set(OPENGL_SOURCES "")
    set(OPENGL_HEADERS "")
endif()
set(SOURCES
    src/Audio.cpp
    src/core.cpp
    src/game_menu.cpp
    src/game_qs.cpp
    src/gfx.cpp
    src/gfx_helpers.cpp
    src/gfx_menu.cpp
    src/gfx_qs.cpp
    src/Grid.cpp
    src/GuiGridCanvas.cpp
    src/GuiScreenManager.cpp
    src/main.cpp
    src/Mode/BaseMode.cpp
    src/MultiEditor.cpp
    src/Path.cpp
    src/PieceDef.cpp
    src/qrs.cpp
    src/random.cpp
    src/replay.cpp
    src/RotationTables.cpp
    src/scores.cpp
    src/Settings.cpp
    src/ShiroPhysoMino.cpp
    src/SPM_Randomizer.cpp
    src/SPM_Spec.cpp
    src/Timer.cpp

    src/SGUIL/SGUIL_GuiButton.cpp
    src/SGUIL/SGUIL_GuiDropDownList.cpp
    src/SGUIL/SGUIL_GuiOptionButton.cpp
    src/SGUIL/SGUIL_GuiTextField.cpp
    src/SGUIL/SGUIL_GuiWindow.cpp
    src/SGUIL/SGUIL.cpp

    ${OPENGL_SOURCES})

set(HEADERS
    src/Audio.hpp
    src/core.h
    src/Debug.hpp
    src/fonts.h
    src/game_menu.h
    src/game_qs.h
    src/gfx.h
    src/gfx_helpers.hpp
    src/gfx_menu.h
    src/gfx_qs.h
    src/gfx_structures.h
    src/Grid.hpp
    src/GuiGridCanvas.hpp
    src/GuiScreenManager.hpp
    src/images.h
    src/Mode/BaseMode.hpp
    src/MultiEditor.hpp
    src/music.h
    src/Path.hpp
    src/PieceDef.hpp
    src/Player/BasePlayer.hpp
    src/qrs.h
    src/QRS.hpp
    src/random.h
    src/replay.h
    src/RotationTables.hpp
    src/scores.h
    src/Settings.hpp
    src/sfx.h
    src/ShiroPhysoMino.hpp
    src/SPM_Randomizer.hpp
    src/SPM_Spec.hpp
    src/stringtools.hpp
    src/TGM.hpp
    src/Timer.hpp

    src/SGUIL/SGUIL.hpp

    ${OPENGL_HEADERS})
source_group("Header Files" FILES ${HEADERS})

add_executable(Shiromino ${SOURCES} ${HEADERS})

target_include_directories(Shiromino SYSTEM PUBLIC third-party)

if(${VCPKG_TOOLCHAIN})
    # For whatever reason, the proper include directory for sqlite3 isn't set when
    # using vcpkg and Visual Studio, so the include directory has to be manually
    # added.
    find_path(INCLUDE_DIRS sqlite3.h)
elseif(${PKG_CONFIG_FOUND})
    target_compile_options(Shiromino PUBLIC
        ${SDL2_CFLAGS}
        ${SDL2_image_CFLAGS}
        ${SDL2_mixer_CFLAGS}
        ${sqlite3_CFLAGS}

        ${SDL2_CFLAGS_OTHER}
        ${SDL2_image_CFLAGS_OTHER}
        ${SDL2_mixer_CFLAGS_OTHER}
        ${sqlite3_CFLAGS_OTHER})
    target_link_directories(Shiromino PUBLIC
        ${SDL2_LIBRARY_DIRS}
        ${SDL2_image_LIBRARY_DIRS}
        ${SDL2_mixer_LIBRARY_DIRS}
        ${sqlite3_LIBRARY_DIRS})
    set(INCLUDE_DIRS
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_image_INCLUDE_DIRS}
        ${SDL2_mixer_INCLUDE_DIRS}
        ${sqlite3_INCLUDE_DIRS})
endif()

target_include_directories(Shiromino PUBLIC ${PROJECT_BINARY_DIR} PRIVATE ${INCLUDE_DIRS})

set(CMAKE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(Shiromino PRIVATE ${LIBRARIES})
