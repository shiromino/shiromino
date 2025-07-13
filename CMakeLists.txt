cmake_minimum_required(VERSION 3.19)

# disable due to clang-scan-deps messing up includes
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 0)

project(shiromino
	DESCRIPTION "A fast-paced puzzle game with roots in the arcade."
	HOMEPAGE_URL "https://github.com/shiromino/shiromino"
	LANGUAGES CXX
	VERSION 0.2.1
)
set(GAME_VERSION "${PROJECT_VERSION}" CACHE STRING "The version of the game.")
set(GAME_EXECUTABLE ${PROJECT_NAME})
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(PORTABLE "Selects whether the build should be \"portable\", where all files are bundled together in one distributable folder/package.")
option(APPIMAGE_BASE_PATH "Selects whether an AppImage build should use the directory where the AppImage is located for the base path, instead of the the contents of the AppImage itself.")


#
# Basic project layout
#
file(GLOB_RECURSE GAME_SOURCES src/*.cc src/*.h)
# Remove main.c/main.cpp from the list to avoid multiple main() functions
list(FILTER GAME_SOURCES EXCLUDE REGEX "src/main\\.cc$")

add_library(shiromino_lib STATIC ${GAME_SOURCES} third-party/PDINI.h)
target_include_directories(shiromino_lib PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
target_include_directories(shiromino_lib SYSTEM PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/third-party"
)

add_executable(shiromino "src/main.cc")
if(PORTABLE AND APPLE)
	set_target_properties(shiromino PROPERTIES MACOSX_BUNDLE TRUE)
	target_sources(shiromino PRIVATE "${CMAKE_SOURCE_DIR}/icon.icns")
	set_source_files_properties("${CMAKE_SOURCE_DIR}/icon.icns" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
elseif(WIN32 AND MSVC)
	target_sources(shiromino PRIVATE "${CMAKE_SOURCE_DIR}/cmake/icon.rc")
endif()
target_include_directories(${GAME_EXECUTABLE} PRIVATE src)
target_include_directories(${GAME_EXECUTABLE} SYSTEM PRIVATE third-party)

target_link_libraries(shiromino PRIVATE shiromino_lib)

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
target_sources(shiromino_lib
	PRIVATE
		"${EXECUTABLE_DYNAMIC_HEADER_DIRECTORY}/definitions.h"
		"${EXECUTABLE_DYNAMIC_HEADER_DIRECTORY}/Version.h"
)
target_include_directories(shiromino_lib PRIVATE "${EXECUTABLE_DYNAMIC_HEADER_DIRECTORY}")


#
# External git project management
#
include(FetchContent)
FetchContent_Declare(pdbm_font
	GIT_REPOSITORY https://github.com/nightmareci/PDBMFont.git
	GIT_TAG f16abc76419f2df31c8c3f0642bedaad99201cda
)
# We don't use FetchContent_MakeAvailable for PDBMFont nor nanotime. The reason
# is that those projects' build pipelines don't cleanly export any targets that
# we can use.
FetchContent_GetProperties(pdbm_font)
if(NOT pdbm_font_POPULATED)
	FetchContent_Populate(pdbm_font)
endif()
FetchContent_GetProperties(pdbm_font
	SOURCE_DIR PDBM_SOURCE_DIRECTORY
)

FetchContent_Declare(nanotime
	GIT_REPOSITORY https://github.com/nightmareci/nanotime.git
	GIT_TAG e8b6a59cdc139fa836c85d9ecbd469aabf828c02
)
FetchContent_GetProperties(nanotime)
if(NOT nanotime_POPULATED)
	FetchContent_Populate(nanotime)
endif()
FetchContent_GetProperties(nanotime
	SOURCE_DIR NANOTIME_SOURCE_DIRECTORY
)

target_sources(shiromino_lib
	PRIVATE
		"${PDBM_SOURCE_DIRECTORY}/PDBMFont.hpp"
		"${PDBM_SOURCE_DIRECTORY}/tinyxml2.cpp"
		"${PDBM_SOURCE_DIRECTORY}/tinyxml2.h"
		"${NANOTIME_SOURCE_DIRECTORY}/nanotime.h"
)
target_include_directories(shiromino_lib SYSTEM
	PRIVATE
		"${PDBM_SOURCE_DIRECTORY}"
		"${NANOTIME_SOURCE_DIRECTORY}"
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
set(MINIMUM_SDL2_VERSION 2.0.5)
set(MINIMUM_SDL2_IMAGE_VERSION 2.0.5)
set(MINIMUM_SDL2_MIXER_VERSION 2.0.3)
set(MINIMUM_SQLITE3_VERSION 3.24.0)

# Catch2 is a header library, so we fetch it regardless to build it alongside the game.
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        2b60af89e23d28eefc081bc930831ee9d45ea58b
)
FetchContent_MakeAvailable(Catch2)

option(FETCH_DEPENDENCIES "Selects whether dependencies should be downloaded and built as part of the project build.")
if(FETCH_DEPENDENCIES)
	# We don't need to check package versions here, as the git tags are
	# effectively version numbers already verified as working.

	set(BUILD_SHARED_LIBS FALSE)

	set(SDL_TEST FALSE)
	FetchContent_Declare(
		SDL2
		GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
		GIT_TAG 4761467b2e8cc7db3d6bc98747daca0051858f09
		GIT_PROGRESS TRUE
		OVERRIDE_FIND_PACKAGE
	)

	set(SDL2IMAGE_VENDORED TRUE)
	set(SDL2IMAGE_SAMPLES FALSE)
	set(SDL2IMAGE_INSTALL FALSE)
	FetchContent_Declare(
		SDL2_image
		GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
		GIT_TAG d3c6d5963dbe438bcae0e2b6f3d7cfea23d02829
		GIT_PROGRESS TRUE
	)

	set(SDL2MIXER_VENDORED TRUE)
	set(SDL2MIXER_SAMPLES FALSE)
	set(SDL2MIXER_INSTALL FALSE)
	FetchContent_Declare(
		SDL2_mixer
		GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
		GIT_TAG 6103316427a8479e5027e41ab9948bebfc1c3c19
		GIT_PROGRESS TRUE
	)

	FetchContent_Declare(
		sqlite3-cmake
		GIT_REPOSITORY https://github.com/undisputed-seraphim/sqlite3-cmake.git
		GIT_TAG 4464c04eb025cd5ff27a2952658da691bc18683e
		GIT_PROGRESS TRUE
	)

	FetchContent_MakeAvailable(
		SDL2
		SDL2_image
		SDL2_mixer
		sqlite3-cmake
	)

	FetchContent_GetProperties(
		sqlite3-cmake
		SOURCE_DIR SQLITE3_SOURCE_DIR
	)
	target_include_directories(shiromino_lib PRIVATE "${SQLITE3_SOURCE_DIR}/src")

	target_link_libraries(shiromino_lib
		PRIVATE
			SDL2::SDL2
			SDL2_image
			SDL2_mixer
			sqlite3
	)
elseif(UNIX OR MINGW)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET "sdl2")
	pkg_check_modules(SDL2_IMAGE REQUIRED IMPORTED_TARGET "SDL2_image")
	pkg_check_modules(SDL2_MIXER REQUIRED IMPORTED_TARGET "SDL2_mixer")
	pkg_check_modules(SQLITE3 REQUIRED IMPORTED_TARGET "sqlite3")

	target_link_libraries(shiromino_lib
		PUBLIC
			PkgConfig::SDL2
			PkgConfig::SDL2_IMAGE
			PkgConfig::SDL2_MIXER
			PkgConfig::SQLITE3
	)
else()
	find_package(SDL2 REQUIRED)
	if(SDL2_VERSION VERSION_LESS 2.0.5)
		message(FATAL_ERROR "SDL2 version ${SDL2_VERSION} found but at least ${MINIMUM_SDL2_VERSION} is required.")
	endif()

	find_package(SDL2_image REQUIRED)
	if(SDL2_image_VERSION VERSION_LESS ${MINIMUM_SDL2_IMAGE_VERSION})
		message(FATAL_ERROR "SDL2_image version ${SDL2_image_VERSION} found but at least ${MINIMUM_SDL2_IMAGE_VERSION} is required.")
	endif()

	find_package(SDL2_mixer REQUIRED)
	if(SDL2_mixer_VERSION VERSION_LESS ${MINIMUM_SDL2_MIXER_VERSION})
		message(FATAL_ERROR "SDL2_mixer version ${SDL2_mixer_VERSION} found but at least ${MINIMUM_SDL2_MIXER_VERSION} is required.")
	endif()

	find_package(SQLite3 REQUIRED)
	if(SQLite3_VERSION VERSION_LESS ${MINIMUM_SQLITE3_VERSION})
		message(FATAL_ERROR "SQLite3 version ${SQLite3_VERSION} found but at least ${MINIMUM_SQLITE3_VERSION} is required.")
	endif()

	target_link_libraries(shiromino_lib
		PUBLIC
			SDL2::SDL2
			SDL2_image::SDL2_image
			SDL2_mixer::SDL2_mixer
			SQLite::SQLite3
	)
endif()


#
# On clang and gcc, add a few sanitizers in the `Debug` configuration.
# Support for this is currently only partially available on MSYS2, so we disable it for WIN32.
#
if(CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    target_compile_options(shiromino_lib
        PUBLIC
          $<$<CONFIG:Debug>:-g>
          -O1
          -Wall
          -Wextra
          -Wpedantic
          -Werror
          $<$<AND:$<CONFIG:Debug>,$<BOOL:${UNIX}>>:
              $<$<NOT:$<CXX_COMPILER_ID:GNU>>:
                -fsanitize=address # bug in gcc standard-library: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105616
              >
              -fsanitize=leak
              -fsanitize=undefined
              -fsanitize-address-use-after-scope
              -fsanitize-undefined-trap-on-error
          >
          -fno-omit-frame-pointer
	)

	target_link_libraries(shiromino_lib
		PUBLIC
          $<$<AND:$<CONFIG:Debug>,$<BOOL:${UNIX}>>:
              $<$<NOT:$<CXX_COMPILER_ID:GNU>>:
                -fsanitize=address # bug in gcc standard-library: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105616
              >
              -fsanitize=leak
              -fsanitize=undefined
              -fsanitize-address-use-after-scope
              -fsanitize-undefined-trap-on-error
          >
          -fno-omit-frame-pointer
		)
endif()
if(MSVC)
	# Enable parallel compiling when using MSVC.
	target_compile_options(shiromino_lib PRIVATE "/MP")
endif()
if(UNIX OR MINGW)
	target_compile_definitions(shiromino_lib PRIVATE _POSIX_VERSION=200112L)
endif()


#
# Testing
#
add_executable(tests
	tests/Grid_test.cc
)
target_link_libraries(tests
	PRIVATE
	  Catch2::Catch2WithMain
	  shiromino_lib
)

list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
include(CTest)
include(Catch)
catch_discover_tests(tests)


#
# Installation
#
if(PORTABLE)
	#
	# If the CPACK_COMPONENTS_ALL variable is left as its default value, then
	# other junk gets installed into CPack-generated packages when using
	# FETCH_DEPENDENCIES. So we specify the stuff we want installed as
	# belonging to a component ("Game") and also specify to only package the
	# component we want ("Game"). The "Game" component is also used when
	# building for AppImage, though CPack isn't used when targeting AppImage;
	# the external build script just specifies to only build the "Game"
	# component instead.
	#
	if(APPLE)
		set(CPACK_COMPONENTS_ALL Game)
		install(DIRECTORY data DESTINATION ${GAME_EXECUTABLE} COMPONENT Game)
		install(FILES LICENSE.md DESTINATION ${GAME_EXECUTABLE} COMPONENT Game)
		install(FILES shiromino.ini DESTINATION ${GAME_EXECUTABLE} COMPONENT Game)
		install(TARGETS ${GAME_EXECUTABLE} BUNDLE DESTINATION ${GAME_EXECUTABLE} COMPONENT Game)

		set(MACOSX_BUNDLE_BUNDLE_NAME ${GAME_EXECUTABLE})
		set(MACOSX_BUNDLE_GUI_IDENTIFIER io.github.shiromino.shiromino)
		set(MACOSX_BUNDLE_ICON_FILE icon)
		set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")
		set(MACOSX_BUNDLE_COPYRIGHT "Copyright © 2015–2022 The shiromino team")
		set(SDL_FILESYSTEM_BASE_DIR_TYPE parent)
		set_target_properties(${GAME_EXECUTABLE} PROPERTIES
			BUNDLE True
			MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/cmake/MacOSXBundleInfo.plist.in"
		)

		set(CPACK_APPLE_CERT_NAME - CACHE STRING "The name of the Apple supplied code signing certificate to use. Defaults to adhoc signing (\"-\")")

		if(CPACK_APPLE_CERT_NAME STREQUAL -)
			set(CPACK_ENTITLEMENTS_FILE "${CMAKE_SOURCE_DIR}/cmake/macos-entitlements-adhoc.xml")
		else()
			set(CPACK_ENTITLEMENTS_FILE "${CMAKE_SOURCE_DIR}/cmake/macos-entitlements-identity-required.xml")
		endif()

		set(CPACK_DMG_VOLUME_NAME ${GAME_EXECUTABLE})
		set(CPACK_PACKAGE_VENDOR "The shiromino team")

		set(CPACK_PACKAGE_FILE_NAME "${GAME_EXECUTABLE}-macos")
		foreach(ARCH ${CMAKE_OSX_ARCHITECTURES})
			string(APPEND CPACK_PACKAGE_FILE_NAME "-${ARCH}")
		endforeach()
		string(APPEND CPACK_PACKAGE_FILE_NAME "-${PROJECT_VERSION}")

		set(CPACK_GENERATOR DragNDrop)
		set(CPACK_POST_BUILD_SCRIPTS "${CMAKE_SOURCE_DIR}/cmake/CPackPostBuildMacOS.cmake")
		include("${CMAKE_SOURCE_DIR}/cmake/InstallRuntimeDependenciesMacOS.cmake" REQUIRED)
		include(CPack)
	elseif(WIN32 AND MSVC)
		set(CPACK_ARCHIVE_COMPONENT_INSTALL TRUE)
		set(CPACK_COMPONENTS_ALL Game)
		# The CPack archive generator can be instructed to generate specific
		# filenames for the archives of individual components as done here, but
		# the component name part of the variable must be an all uppercase
		# version of the component's name.
		if (CMAKE_VS_PLATFORM_NAME STREQUAL Win32)
			set(CPACK_ARCHIVE_GAME_FILE_NAME "${GAME_EXECUTABLE}-windows-x86-${PROJECT_VERSION}")
		else()
			set(CPACK_ARCHIVE_GAME_FILE_NAME "${GAME_EXECUTABLE}-windows-${CMAKE_VS_PLATFORM_NAME}-${PROJECT_VERSION}")
		endif()

		install(DIRECTORY data DESTINATION "${CPACK_ARCHIVE_GAME_FILE_NAME}" COMPONENT Game)
		install(FILES LICENSE.md DESTINATION "${CPACK_ARCHIVE_GAME_FILE_NAME}" COMPONENT Game)
		install(FILES shiromino.ini DESTINATION "${CPACK_ARCHIVE_GAME_FILE_NAME}" COMPONENT Game)
		install(TARGETS ${GAME_EXECUTABLE} RUNTIME DESTINATION "${CPACK_ARCHIVE_GAME_FILE_NAME}" COMPONENT Game)

		set(CPACK_GENERATOR ZIP)
		if(NOT FETCH_DEPENDENCIES)
			include("${CMAKE_SOURCE_DIR}/cmake/InstallRuntimeDependenciesMSVC.cmake" REQUIRED)
		endif()
		include(CPack)
	elseif(APPIMAGE_BASE_PATH)
		include(GNUInstallDirs)
		install(TARGETS ${GAME_EXECUTABLE} RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT Game)
	else()
		install(DIRECTORY data DESTINATION ${GAME_EXECUTABLE})
		install(FILES LICENSE.md DESTINATION ${GAME_EXECUTABLE})
		install(FILES shiromino.ini DESTINATION ${GAME_EXECUTABLE})
		install(TARGETS ${GAME_EXECUTABLE} BUNDLE DESTINATION ${GAME_EXECUTABLE})
	endif()
elseif(UNIX OR MINGW)
	include(GNUInstallDirs)
	install(DIRECTORY data DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")
	install(FILES LICENSE.md DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")
	install(FILES shiromino.ini TYPE SYSCONF)
	install(TARGETS ${GAME_EXECUTABLE} RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()
