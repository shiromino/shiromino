cmake_minimum_required(VERSION 3.15)
project(shiromino
	DESCRIPTION "A fast-paced puzzle game with roots in the arcade."
	HOMEPAGE_URL "https://github.com/shiromino/shiromino"
	LANGUAGES CXX
	VERSION 0.1.0
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
# Suppress the `ZERO_CHECK` target
#
set(CMAKE_SUPPRESS_REGENERATION ON)
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
	#
	# Apple has officially deprecated OpenGL.
	# After some testing, it turns out that even if OpenGL is found, we can not rely on the fact that its initialization will work.
	# So to make things work out of the box, let's disable OpenGL on macOS by default.
	#
	if(NOT APPLE)
		set(ENABLE_OPENGL_INTERPOLATION ON CACHE BOOL "Enables support for the `INTERPOLATE` option in `shiromino.ini`.")
	endif()
endif()
if(ENABLE_OPENGL_INTERPOLATION)
	set(MINIMUM_SDL2_VERSION 2.0.10)
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
		src/Asset/Asset.cc
		src/Asset/Asset.h
		src/Asset/Font.cc
		src/Asset/Font.h
		src/Asset/Image.cc
		src/Asset/Image.h
		src/Asset/Music.cc
		src/Asset/Music.h
		src/Asset/Sfx.cc
		src/Asset/Sfx.h
		src/AssetStore.h
		src/core.cc
		src/CoreState.h
		src/Debug.h
		src/DisplayMode.h
		src/fonts.h
		src/Game.cc
		src/Game.h
		src/GameType.h
		src/game_menu.cc
		src/game_menu.h
		src/game_qs.cc
		src/game_qs.h
		src/Video/Animation.cc
		src/Video/Animation.h
		src/Video/Background.cc
		src/Video/Background.h
		src/Video/Gfx.cc
		src/Video/Gfx.h
		src/Video/MessageEntity.cc
		src/Video/MessageEntity.h
		src/Video/Screen.cc
		src/Video/Screen.h
		src/Video/TextGraphic.cc
		src/Video/TextGraphic.h
		src/Main/Main.cc
		src/Main/Startup.cc
		src/Main/Startup.h
		src/gfx_old.cc
		src/gfx_old.h
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
		src/Input/ControllerBindings.cc
		src/Input/ControllerBindings.h
		src/Input/KeyBindings.cc
		src/Input/KeyBindings.h
		src/Input/KeyFlags.cc
		src/Input/KeyFlags.h
		src/Input/Mouse.cc
		src/Input/Mouse.h
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
		src/OS.h
		src/OS.cc
		src/PieceDefinition.cc
		src/PieceDefinition.h
		src/Player.h
		src/Player/BasePlayer.h
		src/QRS.cc
		src/QRS0.h
		src/QRS1.h
		src/random.cc
		src/random.h
		src/Records.cc
		src/Records.h
		src/RefreshRates.h
		src/replay.cc
		src/replay.h
		src/RotationTables.cc
		src/RotationTables.h
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
if(PKG_CONFIG_FOUND AND NOT VCPKG_TOOLCHAIN)
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
	if(VCPKG_TOOLCHAIN)
		#
		# When using dynamically linked libraries, vcpkg doesn't automatically copy over the vorbis library needed to play ogg files with SDL_mixer,
		# so they have to be explicitly added as dependencies to shiromino to work around that bug.
		#
		find_library(VORBISFILE_LIBRARY vorbisfile REQUIRED)
		target_link_libraries(${GAME_EXECUTABLE} PUBLIC ${VORBISFILE_LIBRARY})
		add_compile_definitions(VCPKG_TOOLCHAIN)
		set(VCPKG_CRT_LINKAGE dynamic)
		set(VCPKG_LIBRARY_LINKAGE dynamic)
	endif()
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
file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}/shiromino.ini" NATIVE_CONFIGURATION_PATH)
set_property(
	TARGET ${GAME_EXECUTABLE}
	PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "--configuration-file ${NATIVE_CONFIGURATION_PATH}"
)
#
# Unsurprisingly, Windows is unusable by default. Let's fix Microsoft's "compiler".
#
if(MSVC)
	#
	# On Windows, doing this is needed to get rid of the `cmd.exe` process that spawns with the game.
	# Usually, you could just compile against `WinMain` directly, but as SDL2 already defines it in order to call `main`,
	# we're forced to provide `main`, which in turn spawns `cmd.exe` on Windows.
	# While that may be useful for debugging, it's very much undesired in actual release builds, so let's disable it here.
	#
	target_link_options(${GAME_EXECUTABLE}
		PRIVATE
			$<$<NOT:$<CONFIG:Debug>>:
				/ENTRY:mainCRTStartup
				/SUBSYSTEM:WINDOWS
			>
	)
	#
	# On Windows, MSVC defines lowercase macros in order to break things, err, I mean, "for backward compatibility".
	# For instance, using `std::max` will give you a syntax error, a truly excellent default behavior.
	# Of course, this little block game should account for Microsoft's historical mistakes.
	#
	add_definitions(-DNOMINMAX)
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
install(DIRECTORY data DESTINATION share/${PROJECT_NAME})
install(FILES shiromino.ini TYPE SYSCONF)
install(TARGETS ${GAME_EXECUTABLE} RUNTIME)