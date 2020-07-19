cmake_minimum_required(VERSION 3.15)
project(shiromino
	DESCRIPTION "A fast-paced puzzle game with roots in the arcade."
	HOMEPAGE_URL "https://github.com/shiromino/shiromino"
	LANGUAGES CXX
	VERSION 0.1.0
)
set(GAME_EXECUTABLE ${PROJECT_NAME})
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
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
# Suppress the `ZERO_CHECK` target
#
set(CMAKE_SUPPRESS_REGENERATION true)
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
# Support for OpenGL
#
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL)
if(OPENGL_FOUND)
	set(ENABLE_OPENGL_INTERPOLATION ON CACHE BOOL "Enables support for the `INTERPOLATE` option in `game.ini`.")
endif()
if(ENABLE_OPENGL_INTERPOLATION)
	find_package(OpenGL REQUIRED)
	target_sources(${GAME_EXECUTABLE}
		PRIVATE
			third-party/glad/glad.cc
			third-party/glad/glad.h
			third-party/glad/khrplatform.h
	)
	target_link_libraries(${GAME_EXECUTABLE} PRIVATE OpenGL::GL)
endif()
#
# Sources needed to build the game
#
target_sources(${GAME_EXECUTABLE}
	PRIVATE
		src/AssetStore.h
		src/Audio.cc
		src/Audio.h
		src/core.cc
		src/CoreState.h
		src/Debug.h
		src/DisplayMode.h
		src/fonts.h
		src/game_menu.cc
		src/game_menu.h
		src/game_qs.cc
		src/game_qs.h
		src/gfx.cc
		src/gfx.h
		src/gfx_helpers.cc
		src/gfx_helpers.h
		src/gfx_menu.cc
		src/gfx_menu.h
		src/gfx_qs.cc
		src/gfx_qs.h
		src/gfx_structures.h
		src/Grid.cc
		src/Grid.h
		src/GuiGridCanvas.cc
		src/GuiGridCanvas.h
		src/GuiScreenManager.cc
		src/GuiScreenManager.h
		src/images.h
		src/Input.h
		src/main.cc
		src/Menu/ActionOption.h
		src/Menu/ElementType.h
		src/Menu/GameArguments.h
		src/Menu/GameMultiOption.h
		src/Menu/GameOption.h
		src/Menu/MetaGameOption.h
		src/Menu/MultiOption.h
		src/Menu/Option.cc
		src/Menu/Option.h
		src/Menu/TextOption.h
		src/Menu/ToggleOption.h
		src/Mode/BaseMode.cc
		src/Mode/BaseMode.h
		src/MultiEditor.cc
		src/MultiEditor.h
		src/music.h
		src/PieceDefinition.cc
		src/PieceDefinition.h
		src/Player.h
		src/Player/BasePlayer.h
		src/QRS.cc
		src/QRS0.h
		src/QRS1.h
		src/random.cc
		src/random.h
		src/RecordList.cc
		src/RecordList.h
		src/RefreshRates.h
		src/replay.cc
		src/replay.h
		src/RotationTables.cc
		src/RotationTables.h
		src/sfx.h
		src/SGUIL/SGUIL.cc
		src/SGUIL/SGUIL.h
		src/SGUIL/SGUIL_GuiButton.cc
		src/SGUIL/SGUIL_GuiTextField.cc
		src/SGUIL/SGUIL_GuiWindow.cc
		src/SPM_Randomizer.cc
		src/SPM_Randomizer.h
		src/SPM_Spec.cc
		src/SPM_Spec.h
		src/Settings.cc
		src/Settings.h
		src/ShiroPhysoMino.cc
		src/ShiroPhysoMino.h
		src/stringtools.h
		src/TGM.h
		src/Timer.cc
		src/Timer.h
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
# TODO: Port this to CONFIG-mode `find_package` calls, see if we can drop `PkgConfig` entirely.
#
find_package(PkgConfig)
if(PKG_CONFIG_FOUND AND NOT VCPKG_TOOLCHAIN)
	pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
	pkg_check_modules(SDL2_image REQUIRED IMPORTED_TARGET SDL2_image)
	pkg_check_modules(SDL2_mixer REQUIRED IMPORTED_TARGET SDL2_mixer)
	pkg_check_modules(sqlite3 REQUIRED IMPORTED_TARGET sqlite3)
	target_link_libraries(${GAME_EXECUTABLE} PRIVATE
		m
		${CMAKE_DL_LIBS}
		${SDL2_LIBRARIES}
		${SDL2_image_LIBRARIES}
		${SDL2_mixer_LIBRARIES}
		${sqlite3_LIBRARIES}
	)
	if(APPLE)
		# Homebrew doesn't appear to append entries to `LD_LIBRARY_PATH`, so we have to do that ourselves.
		target_link_directories(${GAME_EXECUTABLE}
			PRIVATE
				"${SDL2_LIBRARY_DIRS}"
				"${SDL2_image_LIBRARY_DIRS}"
				"${SDL2_mixer_LIBRARY_DIRS}"
				"${sqlite3_LIBRARY_DIRS}"
		)
	endif()
	target_include_directories(${GAME_EXECUTABLE} SYSTEM PRIVATE "${SDL2_INCLUDE_DIRS}")
else()
	find_package(SDL2 REQUIRED)
	find_package(sdl2-image REQUIRED)
	find_package(sdl2-mixer REQUIRED)
	find_package(SQLite3 REQUIRED)
	target_link_libraries(${GAME_EXECUTABLE}
		PRIVATE
			SDL2::SDL2
			SDL2::SDL2main
			SDL2::SDL2_image
			SDL2::SDL2_mixer
			SQLite::SQLite3
	)
	if(VCPKG_TOOLCHAIN)
		# When using dynamically linked libraries, vcpkg doesn't automatically copy over the vorbis library needed to play ogg files with SDL_mixer,
		# so they have to be explicitly added as dependencies to shiromino to work around that bug.
		find_library(VORBISFILE_LIBRARY vorbisfile REQUIRED)
		target_link_libraries(${GAME_EXECUTABLE} PRIVATE ${VORBISFILE_LIBRARY})
		add_compile_definitions(VCPKG_TOOLCHAIN)
		set(VCPKG_CRT_LINKAGE dynamic)
		set(VCPKG_LIBRARY_LINKAGE dynamic)
	endif()
endif()
#
# Visual Studio doesn't know which project to start by default, so it defaults to `ALL_BUILD`, which just errors when you try to launch it.
# Therefore, we set the startup project here.
#
set_property(
	DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	PROPERTY VS_STARTUP_PROJECT ${GAME_EXECUTABLE}
)
#
# Visual Studio won't find the assets by default, since they're nowhere to be found in the build directory.
#
# However, it allows us to specify command-line flags for the debugger.
# We use this to specify the configuration file.
#
file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}/game.ini" NATIVE_CONFIGURATION_PATH)
set_property(
	TARGET ${GAME_EXECUTABLE}
	PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "--configuration-file ${NATIVE_CONFIGURATION_PATH}"
)
#
# On clang and gcc, add a few sanitizers in the `Debug` configuration.
# Support for this is currently only partially available on MSYS2, so we disable it for WIN32.
#
if(CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL GNU)
	if(CMAKE_BUILD_TYPE STREQUAL Debug AND NOT MINGW)
		target_link_libraries(${GAME_EXECUTABLE}
			PRIVATE
				-fsanitize=address,leak,pointer-compare,pointer-subtract,undefined
				-fsanitize-address-use-after-scope
				-fsanitize-undefined-trap-on-error
		)
	endif()
endif()
#
# Installation
#
install(DIRECTORY assets DESTINATION usr/share/${PROJECT_NAME})
install(TARGETS ${GAME_EXECUTABLE} RUNTIME)