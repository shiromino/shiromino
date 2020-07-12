cmake_minimum_required(VERSION 3.10)

project(Shiromino VERSION 0.1 LANGUAGES CXX)
set(CMAKE_VERBOSE_MAKEFILE on)

if(${VCPKG_TOOLCHAIN})
    set(VCPKG_CRT_LINKAGE dynamic)
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(${OPENGL_INTERPOLATION})
    find_package(OpenGL REQUIRED)
    set(OPENGL_LIBRARY OpenGL::GL)
else()
    set(OPENGL_LIBRARY "")
endif()

find_package(PkgConfig)
if(${PKG_CONFIG_FOUND} AND NOT MSVC)
    pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
    pkg_check_modules(SDL2_image REQUIRED IMPORTED_TARGET SDL2_image)
    pkg_check_modules(SDL2_mixer REQUIRED IMPORTED_TARGET SDL2_mixer)
    pkg_check_modules(sqlite3 REQUIRED IMPORTED_TARGET sqlite3)
    set(LIBRARIES
        m
        ${SDL2_LIBRARIES}
        ${SDL2_image_LIBRARIES}
        ${SDL2_mixer_LIBRARIES}
        ${sqlite3_LIBRARIES}
        ${OPENGL_LIBRARY}
        ${CMAKE_DL_LIBS}
    )
else()
    find_package(SDL2 REQUIRED)
    find_package(sdl2-image REQUIRED)
    find_package(sdl2-mixer REQUIRED)
    find_package(sqlite3 REQUIRED)
    if(${VCPKG_TOOLCHAIN})
        # When using dynamically linked libraries, Vcpkg doesn't automatically
        # copy over the vorbis library needed to play ogg files with SDL_mixer,
        # so they have to be explicitly added as dependencies to Shiromino to
        # work around that bug.
        find_library(VORBISFILE_LIBRARY vorbisfile REQUIRED)
        set(LIBRARIES
            SDL2::SDL2 SDL2::SDL2main
            SDL2::SDL2_image
            SDL2::SDL2_mixer
            SQLite::SQLite3
            ${OPENGL_LIBRARY}
            ${VORBISFILE_LIBRARY}
        )
        add_compile_definitions(VCPKG_TOOLCHAIN)
    else()
        set(LIBRARIES
            m
            SDL2::SDL2 SDL2::SDL2main
            SDL2::SDL2_image
            SDL2::SDL2_mixer
            SQLite::SQLite3
            ${OPENGL_LIBRARY}
        )
    endif()
endif()

configure_file(include/Config.h.in Config.h @ONLY)

if(${OPENGL_INTERPOLATION})
    set(OPENGL_SOURCES third-party/glad.cc)
    set(OPENGL_HEADERS
        third-party/glad.h
        third-party/khrplatform.h)
else()
    set(OPENGL_SOURCES "")
    set(OPENGL_HEADERS "")
endif()
set(SOURCES
    src/Audio.cc
    src/core.cc
    src/game_menu.cc
    src/game_qs.cc
    src/gfx_helpers.cc
    src/gfx_menu.cc
    src/gfx_qs.cc
    src/gfx.cc
    src/Grid.cc
    src/GuiGridCanvas.cc
    src/GuiScreenManager.cc
    src/main.cc
    src/Menu/Option.cc
    src/Mode/BaseMode.cc
    src/MultiEditor.cc
    src/PieceDefinition.cc
    src/QRS.cc
    src/random.cc
    src/RecordList.cc
    src/replay.cc
    src/RotationTables.cc
    src/Settings.cc
    src/ShiroPhysoMino.cc
    src/SPM_Randomizer.cc
    src/SPM_Spec.cc
    src/Timer.cc
    src/SGUIL/SGUIL_GuiButton.cc
    src/SGUIL/SGUIL_GuiTextField.cc
    src/SGUIL/SGUIL_GuiWindow.cc
    src/SGUIL/SGUIL.cc
    ${OPENGL_SOURCES}
)
set(HEADERS
    src/AssetStore.h
    src/Audio.h
    src/CoreState.h
    src/Debug.h
    src/DisplayMode.h
    src/fonts.h
    src/game_menu.h
    src/game_qs.h
    src/gfx.h
    src/gfx_helpers.h
    src/gfx_menu.h
    src/gfx_qs.h
    src/gfx_structures.h
    src/Grid.h
    src/GuiGridCanvas.h
    src/GuiScreenManager.h
    src/images.h
    src/Input.h
    src/Menu/ActionOption.h
    src/Menu/ElementType.h
    src/Menu/GameArguments.h
    src/Menu/GameMultiOption.h
    src/Menu/GameOption.h
    src/Menu/MetaGameOption.h
    src/Menu/MultiOption.h
    src/Menu/Option.h
    src/Menu/TextOption.h
    src/Menu/ToggleOption.h
    src/Mode/BaseMode.h
    src/MultiEditor.h
    src/music.h
    src/PieceDefinition.h
    src/Player.h
    src/Player/BasePlayer.h
    src/QRS0.h
    src/QRS1.h
    src/RecordList.h
    src/random.h
    src/replay.h
    src/RefreshRates.h
    src/RotationTables.h
    src/Settings.h
    src/sfx.h
    src/ShiroPhysoMino.h
    src/SPM_Randomizer.h
    src/SPM_Spec.h
    src/stringtools.h
    src/TGM.h
    src/Timer.h
    src/SGUIL/SGUIL.h
    ${OPENGL_HEADERS}
)
source_group("Header Files" FILES ${HEADERS})
add_executable(Shiromino ${SOURCES} ${HEADERS})
target_include_directories(Shiromino SYSTEM PUBLIC src third-party)
if(${VCPKG_TOOLCHAIN})
    # For whatever reason, the proper include directory for SQLite3 isn't set when
    # using Vcpkg and Visual Studio, so the include directory has to be manually
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

target_link_libraries(Shiromino PRIVATE
    ${LIBRARIES}
#     -fsanitize=address,leak,pointer-compare,pointer-subtract,undefined -fsanitize-address-use-after-scope -fsanitize-undefined-trap-on-error
)