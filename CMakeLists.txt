cmake_minimum_required(VERSION 3.15)
project(shiromino
	DESCRIPTION "A fast-paced puzzle game with roots in the arcade."
	HOMEPAGE_URL "https://github.com/shiromino/shiromino"
	LANGUAGES CXX
	VERSION 0.2.1
)
set(GAME_EXECUTABLE ${PROJECT_NAME})
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#
# Basic project layout
#
add_executable(${GAME_EXECUTABLE})
target_include_directories(${GAME_EXECUTABLE} PRIVATE src)
target_include_directories(${GAME_EXECUTABLE} SYSTEM PRIVATE third-party)
#
# Put the git hash into the game if possible
#
find_package(Git)
if(GIT_FOUND)
	set(GIT_COMMIT_HASH unknown)
	execute_process(COMMAND
		${GIT_EXECUTABLE} describe --always --dirty=*
		ERROR_QUIET
		OUTPUT_VARIABLE GIT_COMMIT_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	)
endif()
#
# Dynamic headers
#
set(EXECUTABLE_DYNAMIC_HEADER_DIRECTORY "${CMAKE_BINARY_DIR}/${GAME_EXECUTABLE}-dynamic/include")
configure_file(include/definitions.h.in "${EXECUTABLE_DYNAMIC_HEADER_DIRECTORY}/definitions.h" @ONLY)
configure_file(include/Version.h.in "${EXECUTABLE_DYNAMIC_HEADER_DIRECTORY}/Version.h" @ONLY)
target_sources(${GAME_EXECUTABLE}
	PRIVATE
		"${EXECUTABLE_DYNAMIC_HEADER_DIRECTORY}/definitions.h"
		"${EXECUTABLE_DYNAMIC_HEADER_DIRECTORY}/Version.h"
)
target_include_directories(${GAME_EXECUTABLE} PRIVATE "${EXECUTABLE_DYNAMIC_HEADER_DIRECTORY}")
#
# Sources needed to build the game
#
target_sources(${GAME_EXECUTABLE}
	PRIVATE
		src/asset/Asset.cc
		src/asset/Asset.h
		src/asset/Font.cc
		src/asset/Font.h
		src/asset/Image.cc
		src/asset/Image.h
		src/asset/Music.cc
		src/asset/Music.h
		src/asset/Sfx.cc
		src/asset/Sfx.h
		src/AssetStore.h
		src/CoreState.h
		src/Credits.cc
		src/Credits.h
		src/Debug.h
		src/DisplayMode.h
		src/Game.cc
		src/Game.h
		src/GameType.h
		src/Grid.cc
		src/Grid.h
		src/input/ControllerBindings.cc
		src/input/ControllerBindings.h
		src/input/KeyBindings.cc
		src/input/KeyBindings.h
		src/input/KeyFlags.cc
		src/input/KeyFlags.h
		src/input/Mouse.cc
		src/input/Mouse.h
		src/main.cc
		src/menu/ActionOption.h
		src/menu/ElementType.h
		src/menu/GameArguments.h
		src/menu/GameMultiOption.h
		src/menu/GameOption.h
		src/menu/MetaGameOption.h
		src/menu/MultiOption.h
		src/menu/Option.cc
		src/menu/Option.h
		src/menu/TextOption.h
		src/menu/ToggleOption.h
		src/OS.cc
		src/OS.h
		src/PieceDefinition.cc
		src/PieceDefinition.h
		src/Player.h
		src/QRS.cc
		src/QRS0.h
		src/QRS1.h
		src/Records.cc
		src/Records.h
		src/RefreshRates.h
		src/RotationTables.cc
		src/RotationTables.h
		src/SPM_Randomizer.cc
		src/SPM_Randomizer.h
		src/SPM_Spec.cc
		src/SPM_Spec.h
		src/Settings.cc
		src/Settings.h
		src/ShiroPhysoMino.cc
		src/ShiroPhysoMino.h
		src/TGM.h
		src/Timer.cc
		src/Timer.h
		src/video/Animation.cc
		src/video/Animation.h
		src/video/Background.cc
		src/video/Background.h
		src/video/Gfx.cc
		src/video/Gfx.h
		src/video/MessageEntity.cc
		src/video/MessageEntity.h
		src/video/Render.cc
		src/video/Render.h
		src/video/Screen.cc
		src/video/Screen.h
		src/video/TextGraphic.cc
		src/video/TextGraphic.h
		src/core.cc
		src/fonts.h
		src/game_menu.cc
		src/game_menu.h
		src/game_qs.cc
		src/game_qs.h
		src/gfx_helpers.cc
		src/gfx_helpers.h
		src/gfx_menu.cc
		src/gfx_menu.h
		src/gfx_old.cc
		src/gfx_old.h
		src/gfx_qs.cc
		src/gfx_qs.h
		src/gfx_structures.h
		src/gui/Button.cc
		src/gui/GUI.cc
		src/gui/GUI.h
		src/gui/GridCanvas.cc
		src/gui/GridCanvas.h
		src/gui/ScreenManager.cc
		src/gui/ScreenManager.h
		src/gui/TextField.cc
		src/gui/Window.cc
		src/images.h
		src/random.cc
		src/random.h
		src/replay.cc
		src/replay.h
		src/stringtools.h
		third-party/PDINI.h
)
#
# External git project management
#
include(FetchContent)
FetchContent_Declare(pdbm_font
	GIT_REPOSITORY https://github.com/nightmareci/PDBMFont.git
)
# We don't use FetchContent_MakeAvailable here.
# The reason for that is that the project's build pipeline doesn't cleanly export any targets that we can use.
FetchContent_GetProperties(pdbm_font)
if(NOT pdbm_font_POPULATED)
	FetchContent_Populate(pdbm_font)
endif()
FetchContent_GetProperties(pdbm_font
	SOURCE_DIR PDBM_SOURCE_DIRECTORY
)
target_sources(${GAME_EXECUTABLE}
	PRIVATE
		"${PDBM_SOURCE_DIRECTORY}/PDBMFont.hpp"
		"${PDBM_SOURCE_DIRECTORY}/tinyxml2.cpp"
		"${PDBM_SOURCE_DIRECTORY}/tinyxml2.h"
)
target_include_directories(${GAME_EXECUTABLE}
	PRIVATE
		"${PDBM_SOURCE_DIRECTORY}"
)
#
# The "Headers" source group for IDEs
#
get_target_property(EXECUTABLE_SOURCES ${GAME_EXECUTABLE} SOURCES)
set(EXECUTABLE_HEADERS "${EXECUTABLE_SOURCES}")
list(FILTER EXECUTABLE_HEADERS INCLUDE REGEX .h$)
list(JOIN EXECUTABLE_HEADERS " " EXECUTABLE_HEADERS)
source_group("Headers" FILES "${EXECUTABLE_HEADERS}")
#
# The "Third-party" source group for IDEs
#
set(EXECUTABLE_THIRD_PARTY "${EXECUTABLE_SOURCES}")
list(FILTER EXECUTABLE_THIRD_PARTY INCLUDE REGEX third-party)
list(JOIN EXECUTABLE_THIRD_PARTY " " EXECUTABLE_THIRD_PARTY)
source_group("Third-party" FILES "${EXECUTABLE_THIRD_PARTY}")
#
# Dependency management
#
find_package(PkgConfig)
if(NOT DEFINED MINIMUM_SDL2_VERSION)
	set(MINIMUM_SDL2_VERSION 2.0.5)
endif()
set(MINIMUM_SDL2_IMAGE_VERSION 2.0.5)
set(MINIMUM_SDL2_MIXER_VERSION 2.0.3)
set(MINIMUM_SQLITE3_VERSION 3.24.0)
function(target_pkg_config_import TARGET DEPENDENCY)
	string(TOUPPER ${DEPENDENCY} DEPENDENCY_VARIABLE)
	set(MINIMUM_VERSION ${MINIMUM_${DEPENDENCY_VARIABLE}_VERSION})
	pkg_check_modules(${DEPENDENCY_VARIABLE} REQUIRED ${DEPENDENCY}>=${MINIMUM_VERSION})
	target_compile_options(${TARGET} PRIVATE "${${DEPENDENCY_VARIABLE}_CFLAGS}")
	target_compile_options(${TARGET} PRIVATE "${${DEPENDENCY_VARIABLE}_CFLAGS_OTHER}")
	target_include_directories(${TARGET} SYSTEM PRIVATE "${${DEPENDENCY_VARIABLE}_INCLUDE_DIRS}")
	target_link_directories(${TARGET} PRIVATE "${${DEPENDENCY_VARIABLE}_LIBRARY_DIRS}")
	target_link_libraries(${TARGET} PRIVATE "${${DEPENDENCY_VARIABLE}_LIBRARIES}")
	target_link_options(${TARGET} PRIVATE "${${DEPENDENCY_VARIABLE}_LDFLAGS}")
	target_link_options(${TARGET} PRIVATE "${${DEPENDENCY_VARIABLE}_LDFLAGS_OTHER}")
endfunction()
if(PKG_CONFIG_FOUND)
	#
	# Link anything that `pkg-config` can't resolve on its own.
	#
	target_link_libraries(${GAME_EXECUTABLE} PRIVATE
		m
		${CMAKE_DL_LIBS}
	)
	#
	# Let `pkg-config` do its magic.
	#
	set(DEPENDENCIES
		sdl2
		SDL2_image
		SDL2_mixer
		sqlite3
	)
	foreach(DEPENDENCY ${DEPENDENCIES})
		target_pkg_config_import(${GAME_EXECUTABLE} ${DEPENDENCY})
	endforeach()
else()
	#
	# We *could* check for minimum version requirements here, but vcpkg packages don't have any
	# version information attached that we could query for from CMake.
	#
	# According to https://github.com/Microsoft/vcpkg/issues/1681, package versioning in vcpkg hasn't been fully conceptualized yet.
	# For now, we won't check the minimum version requirements here.
	#
	find_package(SDL2 REQUIRED)
	find_package(sdl2-image REQUIRED)
	find_package(sdl2-mixer REQUIRED)
	find_package(SQLite3 REQUIRED)
	target_link_libraries(${GAME_EXECUTABLE}
		PUBLIC
			SDL2::SDL2
			SDL2::SDL2main
			SDL2::SDL2_image
			SDL2::SDL2_mixer
			SQLite::SQLite3
	)
endif()
#
# On distributions with prehistoric packages, clang/gcc are unable to link these libraries themselves.
# NOTE: We should remove this section as soon as debian-stable adds `{clang,g++}-9`.
#
# See https://en.cppreference.com/w/cpp/filesystem
#
if(
	CMAKE_CXX_COMPILER_ID STREQUAL GNU AND
	CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 5.0 AND
	CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1
)
	set(FILE_SYSTEM_LIBRARY_NAME stdc++fs)
elseif(
	CMAKE_CXX_COMPILER_ID STREQUAL Clang AND
	CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 5.0 AND
	CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0
)
	set(FILE_SYSTEM_LIBRARY_NAME c++fs)
endif()
if(DEFINED FILE_SYSTEM_LIBRARY_NAME)
	find_library(FILE_SYSTEM ${FILE_SYSTEM_LIBRARY_NAME}
		PATHS
			/usr/lib/llvm-5/lib
			/usr/lib/llvm-6/lib
			/usr/lib/llvm-7/lib
			/usr/lib/llvm-8/lib
			/usr/lib/gcc/x86_64-linux-gnu/5
			/usr/lib/gcc/x86_64-linux-gnu/6
			/usr/lib/gcc/x86_64-linux-gnu/7
			/usr/lib/gcc/x86_64-linux-gnu/8
			/usr/lib/gcc/x86_64-linux-gnu/9
		REQUIRED
	)
	target_link_libraries(${GAME_EXECUTABLE} PRIVATE ${FILE_SYSTEM})
endif()
#
# On clang and gcc, add a few sanitizers in the `Debug` configuration.
# Support for this is currently only partially available on MSYS2, so we disable it for WIN32.
#
if(CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL GNU)
	if(CMAKE_BUILD_TYPE STREQUAL Debug AND NOT MINGW)
		target_link_libraries(${GAME_EXECUTABLE}
			PRIVATE
				-fsanitize=address,leak,undefined
				-fsanitize-address-use-after-scope
				-fsanitize-undefined-trap-on-error
		)
	endif()
endif()
#
# Installation
#
install(DIRECTORY data DESTINATION usr/share/${PROJECT_NAME})
install(FILES LICENSE.md DESTINATION usr/share/licenses/${PROJECT_NAME})
install(FILES shiromino.ini TYPE SYSCONF)
install(TARGETS ${GAME_EXECUTABLE} RUNTIME DESTINATION usr/bin)